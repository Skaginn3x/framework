#pragma once
#include <async_mqtt/all.hpp>
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

template <class ipc_client_type, class match_client, class mqtt_client_type>
class mqtt_broadcaster {
public:
  mqtt_broadcaster(asio::io_context& ctx,
                   std::string mqtt_address,
                   std::string mqtt_port,
                   std::string mqtt_username,
                   std::string mqtt_password,
                   ipc_client_type& ipc_client,
                   std::shared_ptr<mqtt_client_type> mqtt_client)
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
    asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::detached);

    config_.value()._banned_topics.observe([&, this]([[maybe_unused]] auto& new_conf, [[maybe_unused]] auto& old_conf) {
      banned_signals_.clear();

      for (auto const& con : new_conf) {
        banned_signals_.push_back(con);
      }
      restart();
    });

    asio::co_spawn(mqtt_client_->strand(), tfc::base::exit_signals(ctx), asio::detached);

    banned_signals_ = config_.value()._banned_topics.value();
    load_signals();
  }

private:
  auto receive_packet(async_mqtt::control_packet_type packet_type) -> asio::awaitable<void> {
    for (int i = 0; i < 5; i++) {
      async_mqtt::packet_variant packet_variant;

      try {
        packet_variant = co_await mqtt_client_->recv(asio::use_awaitable);

        if (packet_variant.type() == packet_type) {
          logger_.trace("Received MQTT connack");
          co_return;
        }

      } catch (std::exception& e) {
        logger_.error("Exception: {}", e.what());
        continue;
      }

      co_await asio::steady_timer{ ctx_, std::chrono::seconds{ 1 } }.async_wait(asio::use_awaitable);
    }

    logger_.error("Timed out");
    throw std::exception();
  }

  auto connect_to_broker() -> asio::awaitable<void> {
    while (true) {
      bool exception = false;
      try {
        logger_.trace("Connecting to MQTT broker, closing the previous connection");
        // [[maybe_unused]] auto result = mqtt_client_->close(asio::use_awaitable);

        asio::ip::tcp::socket resolve_sock{ ctx_ };
        asio::ip::tcp::resolver res{ resolve_sock.get_executor() };

        logger_.trace("Resolving IP number");
        asio::ip::tcp::resolver::results_type resolved_ip =
            co_await res.async_resolve(mqtt_host_, mqtt_port_, asio::use_awaitable);

        logger_.trace("Connecting to IP number");
        co_await asio::async_connect(mqtt_client_->next_layer(), resolved_ip, asio::use_awaitable);

        logger_.trace("Performing MQTT handshake");
        co_await mqtt_client_->send(
            async_mqtt::v5::connect_packet{ false,
                                            std::chrono::seconds(100).count(),
                                            async_mqtt::allocate_buffer("cid1"),
                                            async_mqtt::nullopt,
                                            async_mqtt::allocate_buffer(mqtt_username_),
                                            async_mqtt::allocate_buffer(mqtt_password_),
                                            { async_mqtt::property::session_expiry_interval{ UINT_MAX } } },
            asio::use_awaitable);

        logger_.trace("Waiting for MQTT connack");

        co_await receive_packet(async_mqtt::control_packet_type::connack);

        connect_active = false;
        break;
      } catch (std::exception& e) {
        logger_.error("Error Error Error while connecting: {}", e.what());
        exception = true;
      }
      if (exception) {
        logger_.error("Error Error Error while connecting: {}", "exception");
        co_await asio::steady_timer{ ctx_, std::chrono::seconds{ 1 } }.async_wait(asio::use_awaitable);
      }
    }
  }

  auto load_signals() -> void {
    std::vector<tfc::ipc_ruler::signal> signals = get_signals();

    // connecting to all signals
    for (tfc::ipc_ruler::signal& signal : signals) {
      handle_signal(signal);
    }
  }

  // get all signals from the ipc manager
  auto get_signals() -> std::vector<tfc::ipc_ruler::signal> {
    std::vector<tfc::ipc_ruler::signal> signals_on_client;

    // store the found signals in a vector
    ipc_client_.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
      for (const tfc::ipc_ruler::signal& signal : signals) {
        signals_on_client.push_back(signal);
      }
    });

    // give the signals some time to arrive
    ctx_.run_for(std::chrono::milliseconds(20));

    // remove all signals that contain banned strings
    signals_on_client = clean_signals(signals_on_client);

    return signals_on_client;
  }

  // remove all signals that contain banned strings
  auto clean_signals(std::vector<tfc::ipc_ruler::signal> signals) -> std::vector<tfc::ipc_ruler::signal> {
    signals.erase(std::remove_if(signals.begin(), signals.end(),
                                 [&](const tfc::ipc_ruler::signal& signal) {
                                   return std::ranges::any_of(banned_signals_.begin(), banned_signals_.end(),
                                                              [&](const std::string& banned_string) {
                                                                return signal.name.find(banned_string) != std::string::npos;
                                                              });
                                 }),
                  signals.end());

    return signals;
  }

  // auto handle_signal(asio::io_context& ctx, auto& signal) -> void {
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
    asio::co_spawn(mqtt_client_->strand(), slot_coroutine<slot_type, value_type>(*slot, signal.name), asio::detached);
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
      // logger_.trace("waiting for signal: {}", signal_name);
      std::expected<value_type, std::error_code> msg = co_await slot.coro_receive();
      std::string value_string = convert_to_string(msg.value());
      co_await send_message<value_type>(msg, value_string, signal_name);
    }
  }

  template <class value_type>
  auto send_message(std::expected<value_type, std::error_code> msg, std::string value_string, std::string signal_name)
      -> asio::awaitable<void> {
    bool exception = false;

    while (true) {
      exception = false;

      co_await asio::post(mqtt_client_->strand(), asio::use_awaitable);

      if (msg) {
        logger_.trace("sending message: {} to topic: {}", value_string, signal_name);

        try {
          auto result = co_await mqtt_client_->send(
              async_mqtt::v5::publish_packet{ mqtt_client_->acquire_unique_packet_id().value(),
                                              async_mqtt::allocate_buffer(signal_name),
                                              async_mqtt::allocate_buffer(value_string), async_mqtt::qos::at_least_once },

              asio::use_awaitable);

          // co_await receive_packet(async_mqtt::control_packet_type::puback);

          logger_.trace(result.message());

          if (result) {
            exception = true;
            logger_.error("failed to connect to mqtt client: {}", result.message());
            co_await asio::steady_timer{ ctx_, std::chrono::milliseconds{ 100 } }.async_wait(asio::use_awaitable);
            // co_await mqtt_connect();
          } else {
            logger_.trace("message sent successfully");
            break;
          }
        } catch (std::exception& e) {
          logger_.error("exception in send: {}", e.what());
          exception = true;
        }
      }

      if (exception) {
        if (!connect_active) {
          connect_active = true;
          logger_.trace("Exception in send, reconnecting to broker");
          asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::detached);
        }
      }
    }
    co_return;
  }

  asio::io_context& ctx_;
  ipc_client_type ipc_client_;

  std::string service_name_;
  std::string object_path_;
  std::string interface_name_;

  std::shared_ptr<mqtt_client_type> mqtt_client_;

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
  std::vector<std::string> banned_signals_;
  bool connect_active = false;
};
