#include <mqtt/async_client.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iterator>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/unpack_properties.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/to_tuple.hpp>

// using namespace std;
namespace asio = boost::asio;
namespace details = tfc::ipc::details;

struct config {
  tfc::confman::observable<std::vector<std::string>> _banned_topics{};
  struct glaze {
    static constexpr auto value{ glz::object("_banned_topics", &config::_banned_topics) };
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};

template <class ipc_client_type, class match_client>
class mqtt_broadcaster {
public:
  mqtt_broadcaster(asio::io_context& ctx,
                   std::string mqtt_address,
                   std::string mqtt_port,
                   std::string mqtt_username,
                   std::string mqtt_password,
                   ipc_client_type& ipc_client,
                   std::shared_ptr<mqtt::async_client> mqtt_client)
      : ctx_(ctx), ipc_client_(ipc_client), service_name_(tfc::dbus::make_dbus_name("ipc_ruler")),
        object_path_(tfc::dbus::make_dbus_path("ipc_ruler")), interface_name_(tfc::dbus::make_dbus_name("manager")),
        mqtt_client_(std::move(mqtt_client)), mqtt_host_(std::move(mqtt_address)), mqtt_port_(std::move(mqtt_port)),
        mqtt_username_(std::move(mqtt_username)), mqtt_password_(std::move(mqtt_password)), logger_("mqtt_broadcaster"),
        dbus_connection_(std::make_unique<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system())),
        signal_updates_(
            std::make_unique<match_client>(*dbus_connection_,
                                           sdbusplus::bus::match::rules::propertiesChanged(object_path_, interface_name_),
                                           std::bind_front(&mqtt_broadcaster::update_signals, this))),
        config_(ctx, "mqtt_broadcaster") {

    std::cout << "here\n";
    connect_to_broker();
    std::cout << "here\n";

    config_.value()._banned_topics.observe([this]([[maybe_unused]] auto& new_conf, [[maybe_unused]] auto& old_conf) {
      banned_topics_.clear();
      for (auto& topic : new_conf) {
        banned_topics_.push_back(topic);
      }
      restart();
    });
    std::cout << "here\n";

    asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);
    std::cout << "here\n";

    banned_topics_ = config_.value()._banned_topics.value();
    std::cout << "here\n";

    load_signals();
    std::cout << "here\n";
  }

private:
  // will repeatedly try to connect to the broker until successful
  auto connect_to_broker() -> void {
    while (true) {
      try {
        mqtt::token_ptr connection_token = mqtt_client_->connect(mqtt::connect_options_builder().clean_session().finalize());
        connection_token->wait();
        mqtt::message_ptr test_message = mqtt::make_message("connected", "true");
        test_message->set_qos(qos_);
        mqtt_client_->publish(test_message)->wait_for(timeout_);
        break;
      } catch (const mqtt::exception& exc) {
        logger_.error("Failed to connect to MQTT broker: {}", exc.what());
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  auto load_signals() -> void {
    for (tfc::ipc_ruler::signal& signal : signals_on_client) {
      handle_signal(signal);
    }
  }

  auto get_signals() -> void {
    ipc_client_.signals([this]([[maybe_unused]] const std::vector<tfc::ipc_ruler::signal>& signals) {
      for (tfc::ipc_ruler::signal signal : signals) {
        signals_on_client.push_back(signal);
      }
    });

    // give the signals some time to arrive
    ctx_.run_for(std::chrono::milliseconds(100));
  }

  auto clean_signals() -> void {
    signals_on_client.erase(std::remove_if(signals_on_client.begin(), signals_on_client.end(),
                                           [&](const tfc::ipc_ruler::signal& signal) {
                                             return std::ranges::any_of(banned_topics_,
                                                                        banned_topics_,
                                                                        [&](const std::string& banned_string) {
                                                                          return signal.name.find(banned_string) !=
                                                                                 std::string::npos;
                                                                        });
                                           }),
                            signals_on_client.end());
  }

  auto handle_signal(tfc::ipc_ruler::signal& signal) -> void {
    switch (signal.type) {
      case details::type_e::_bool: {
        run_slot<tfc::ipc::details::type_bool, bool>(signal);
        break;
      }
      case details::type_e::_int64_t: {
        run_slot<tfc::ipc::details::type_int, int>(signal);
        break;
      }
      case details::type_e::_uint64_t: {
        run_slot<tfc::ipc::details::type_uint, uint>(signal);
        break;
      }
      case details::type_e::_double_t: {
        run_slot<tfc::ipc::details::type_double, double>(signal);
        break;
      }
      case details::type_e::_string: {
        run_slot<tfc::ipc::details::type_string, std::string>(signal);
        break;
      }
      case details::type_e::_json: {
        run_slot<tfc::ipc::details::type_json, std::string>(signal);
        break;
      }
      case details::type_e::unknown: {
        break;
      }
    }
  }

  // template the types away
  template <typename slot_type, typename value_type>
  auto run_slot(tfc::ipc_ruler::signal& signal) -> void {
    slots_.push_back(std::make_shared<details::slot<slot_type>>(ctx_, signal.name));
    auto& slot = std::get<std::shared_ptr<details::slot<slot_type>>>(slots_.back());
    slot->connect(signal.name);
    asio::co_spawn(ctx_, slot_coroutine<slot_type, value_type>(*slot, signal.name), asio::detached);
  }

  // read all the signals again and restart the coroutine
  auto restart() -> void {
    stop_coroutine_ = true;
    logger_.info("New signal has arrived, reloading signals");
    load_signals();
    stop_coroutine_ = false;
  }

  //
  void update_signals([[maybe_unused]] sdbusplus::message::message& msg) noexcept { restart(); }

  template <typename value_type>
  auto convert_to_string(value_type value) -> std::string {
    if constexpr (std::is_same_v<value_type, bool>) {
      return value ? "true" : "false";
    } else if constexpr (std::is_same_v<value_type, std::string>) {
      return value;
    } else if constexpr (std::is_arithmetic_v<value_type>) {
      return std::to_string(value);
    } else {
      return "unknown";
    }
  }

  template <class slot_type, class value_type>
  auto slot_coroutine(details::slot<slot_type>& slot, std::string signal_name) -> asio::awaitable<void> {
    logger_.info("starting coroutine for signal: {}", signal_name);
    std::replace(signal_name.begin(), signal_name.end(), '.', '/');

    while (!stop_coroutine_) {
      logger_.trace("waiting for signal: {}", signal_name);
      std::expected<value_type, std::error_code> msg = co_await slot.coro_receive();
      if (msg) {
        std::string value_string = convert_to_string(msg.value());
        co_await send_message(value_string, signal_name);
      }
    }
  }

  // template <class value_type>
  auto send_message(std::string value_string, std::string signal_name) -> asio::awaitable<void> {
    bool exception = false;
    while (true) {
      logger_.trace("sending message: {} to topic: {}", value_string, signal_name);
      try {
        mqtt::message_ptr pubmsg = mqtt::make_message(signal_name, value_string);
        pubmsg->set_qos(qos_);
        mqtt_client_->publish(pubmsg)->wait_for(timeout_);
        break;
      } catch (std::exception& e) {
        exception = true;
        logger_.error("exception in send: {}", e.what());
      }

      if (exception) {
        connect_to_broker();
      }
    }
    co_return;
  }

  asio::io_context& ctx_;
  ipc_client_type ipc_client_;

  std::string service_name_;
  std::string object_path_;
  std::string interface_name_;

  std::shared_ptr<mqtt::async_client> mqtt_client_;

  std::string mqtt_host_;
  std::string mqtt_port_;
  std::string mqtt_username_;
  std::string mqtt_password_;
  tfc::logger::logger logger_;

  std::unique_ptr<sdbusplus::asio::connection, std::function<void(sdbusplus::asio::connection*)>> dbus_connection_;
  std::unique_ptr<match_client, std::function<void(match_client*)>> signal_updates_;

  tfc::confman::config<config> config_;

  std::vector<std::variant<std::shared_ptr<details::slot<details::type_bool>>,
                           std::shared_ptr<details::slot<details::type_string>>,
                           std::shared_ptr<details::slot<details::type_int>>,
                           std::shared_ptr<details::slot<details::type_uint>>,
                           std::shared_ptr<details::slot<details::type_double>>,
                           std::shared_ptr<details::slot<details::type_json>>>>
      slots_;

  std::vector<std::unique_ptr<sdbusplus::bus::match::match>> signals_;

  bool stop_coroutine_ = false;

  int qos_ = 1;
  std::chrono::seconds timeout_ = std::chrono::seconds(10);
  std::vector<tfc::ipc_ruler::signal> signals_on_client;

  std::vector<std::string> banned_topics_;
};
