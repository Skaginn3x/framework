// This program reads signals off the ipc-client and publishes changes to the mqtt broker specified in the command line.

#pragma once
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/string_maker.hpp>

namespace asio = boost::asio;
namespace details = tfc::ipc::details;

// File under /etc/tfc/mqtt-broadcaster/def/mqtt_broadcaster.json which lists the signals that are not supposed to be
// published on the mqtt broker. Updating the signals in this file will cause the running program to stop and restart
struct config {
  tfc::confman::observable<std::vector<std::string>> _banned_topics{};
  struct glaze {
    static constexpr auto value{ glz::object("_banned_topics", &config::_banned_topics) };
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};

// In order to ease testing, types are injected into the class
template <class ipc_client_type, class mqtt_client_type>
class mqtt_broadcaster {
public:
  mqtt_broadcaster(asio::io_context& ctx,
                   std::string mqtt_address,
                   std::string mqtt_port,
                   std::string mqtt_username,
                   std::string mqtt_password,
                   ipc_client_type& ipc_client,
                   std::shared_ptr<mqtt_client_type> mqtt_client)
      : object_path_(tfc::dbus::make_dbus_path("ipc_ruler")), interface_name_(tfc::dbus::make_dbus_name("manager")),
        mqtt_host_(std::move(mqtt_address)), mqtt_port_(std::move(mqtt_port)), mqtt_username_(std::move(mqtt_username)),
        mqtt_password_(std::move(mqtt_password)), ctx_(ctx), ipc_client_(ipc_client), mqtt_client_(std::move(mqtt_client)),
        logger_("mqtt_broadcaster"),
        dbus_connection_(std::make_unique<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system())),
        signal_updates_(std::make_unique<sdbusplus::bus::match::match>(
            *dbus_connection_,
            sdbusplus::bus::match::rules::propertiesChanged(object_path_, interface_name_),
            std::bind_front(&mqtt_broadcaster::update_signals, this))),
        config_(ctx, "mqtt_broadcaster") {
    asio::co_spawn(mqtt_client_->strand(), initialize(), asio::detached);
  }

private:
  auto initialize() -> asio::awaitable<void> {
    // Initial connect to the broker
    if (!connect_active_) {
      connect_active_ = true;
      co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
    }

    // When a new list of banned string arrives the program is restarted
    config_.value()._banned_topics.observe([this](auto& new_conf, [[maybe_unused]] auto& old_conf) {
      banned_signals_.clear();
      for (auto const& con : new_conf) {
        banned_signals_.push_back(con);
      }
      restart();
    });

    asio::co_spawn(mqtt_client_->strand(), tfc::base::exit_signals(ctx_), asio::detached);

    banned_signals_ = config_.value()._banned_topics.value();
    load_signals();
  }

  // Connect the mqtt client to the broker. Only a single "thread" can run this at a time. The routine that calls this
  // function must set the connect_active_ flag to true before calling this function and the function sets it to false when
  // finished.
  auto connect_to_broker() -> asio::awaitable<void> {
    while (true) {
      try {
        logger_.trace("Connecting to MQTT broker");
        asio::ip::tcp::socket resolve_sock{ ctx_ };
        asio::ip::tcp::resolver res{ resolve_sock.get_executor() };

        logger_.trace("Resolving IP number");
        asio::ip::tcp::resolver::results_type resolved_ip =
            co_await res.async_resolve(mqtt_host_, mqtt_port_, asio::use_awaitable);

        logger_.trace("Connecting MQTT client socket to IP number of broker");
        co_await asio::async_connect(mqtt_client_->next_layer(), resolved_ip, asio::use_awaitable);

        logger_.trace("Sending Connect request to the MQTT broker");
        co_await mqtt_client_->send(
            async_mqtt::v5::connect_packet{ false,
                                            std::chrono::seconds(100).count(),
                                            async_mqtt::allocate_buffer("cid1"),
                                            async_mqtt::nullopt,
                                            async_mqtt::allocate_buffer(mqtt_username_),
                                            async_mqtt::allocate_buffer(mqtt_password_),
                                            { async_mqtt::property::session_expiry_interval{ UINT_MAX } } },
            asio::use_awaitable);

        logger_.trace("Waiting for MQTT connection acknowledgement");
        co_await mqtt_client_->recv(async_mqtt::filter::match, { async_mqtt::control_packet_type::connack },
                                    asio::use_awaitable);

        connect_active_ = false;
        break;
      } catch (std::exception& e) {
        logger_.error("Error while connecting to MQTT broker: {}", e.what());
      } catch (...) {
        logger_.error("Unknown error while connecting to MQTT broker");
      }

      // If the connection failed, then clear the socket and try again after 1 second.
      logger_.trace("Waiting for 1 second and then trying to connect again");
      co_await asio::co_spawn(mqtt_client_->strand(), mqtt_client_->close(asio::use_awaitable), asio::use_awaitable);
      co_await asio::steady_timer{ ctx_, std::chrono::seconds{ 1 } }.async_wait(asio::use_awaitable);
    }
  }

  // Function which has oversight over the signals
  auto load_signals() -> void {

    // Stop reading the current signals by canceling the slots
    for (auto& slot : slots_) {
      std::visit([](auto& ptr) {
        ptr->cancel();
      }, slot);
    }

    active_signals_.clear();
    get_signals();
    clean_signals();

    for (tfc::ipc_ruler::signal& signal : active_signals_) {
      handle_signal(signal);
    }
  }

  auto get_signals() -> void {
    ipc_client_.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
      for (const tfc::ipc_ruler::signal& signal : signals) {
        active_signals_.push_back(signal);
      }
    });

    ctx_.run_for(std::chrono::milliseconds(20));
  }

  // This function removes signals that are banned
  auto clean_signals() -> void {
    active_signals_.erase(std::remove_if(active_signals_.begin(), active_signals_.end(),
                                         [&](const tfc::ipc_ruler::signal& signal) {
                                           return std::ranges::any_of(banned_signals_.begin(), banned_signals_.end(),
                                                                      [&](const std::string& banned_string) {
                                                                        return signal.name.find(banned_string) !=
                                                                               std::string::npos;
                                                                      });
                                         }),
                          active_signals_.end());
  }

  // This function checks the type of the signal in order to determine how to read from the slot
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

  // This function runs the coroutine for the slot
  template <typename slot_type, typename value_type>
  auto run_slot(tfc::ipc_ruler::signal& signal) -> void {
    slots_.push_back(std::make_shared<details::slot<slot_type>>(ctx_, signal.name));
    auto& slot = std::get<std::shared_ptr<details::slot<slot_type>>>(slots_.back());
    slot->connect(signal.name);
    asio::co_spawn(mqtt_client_->strand(), slot_coroutine<slot_type, value_type>(*slot, signal.name), asio::detached);
  }

  // This function stops the coroutines that are currently running and then restarts them
  auto restart() -> void {
    stop_coroutine_ = true;
    ctx_.run_for(std::chrono::milliseconds(20));
    logger_.info("Signal list has been updated, reloading all signals");
    load_signals();
    stop_coroutine_ = false;
  }

  // This function is called when a signal is received
  void update_signals([[maybe_unused]] sdbusplus::message::message& msg) noexcept { restart(); }

  // This function is called when a value is received and its value needs to be converted to a string
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

  // This function runs a coroutine for a slot, when a new value is received a message is sent to the MQTT broker
  template <class slot_type, class value_type>
  auto slot_coroutine(details::slot<slot_type>& slot, std::string signal_name) -> asio::awaitable<void> {
    logger_.info("Starting coroutine for signal: {}", signal_name);
    std::replace(signal_name.begin(), signal_name.end(), '.', '/');

    while (!stop_coroutine_) {
      std::expected<value_type, std::error_code> msg = co_await slot.async_receive(asio::use_awaitable);
      if (msg) {
        std::string message_value = convert_to_string(msg.value());
        co_await send_message<value_type>(signal_name, message_value);
      }
    }
  }

  // This function sends a message to the MQTT broker. If the send fails then it will try to restart the broker
  template <class value_type>
  auto send_message(std::string signal, std::string value) -> asio::awaitable<void> {
    logger_.trace("Sending message: {} to topic: {}", value, signal);
    co_await asio::post(mqtt_client_->strand(), asio::use_awaitable);

    bool send_failed = false;
    try {
      auto result = co_await mqtt_client_->send(
          async_mqtt::v5::publish_packet{ mqtt_client_->acquire_unique_packet_id().value(),
                                          async_mqtt::allocate_buffer(signal), async_mqtt::allocate_buffer(value),
                                          async_mqtt::qos::at_least_once },
          asio::use_awaitable);

      if (result) {
        logger_.error("Send failed: {}", result.message());
        send_failed = true;
      } else {
        logger_.trace("Message sent successfully");
      }
    } catch (std::exception& e) {
      logger_.error("Exception in send: {}", e.what());
      send_failed = true;
    }

    if (send_failed) {
      logger_.trace("Exception in send, reconnecting to broker");
      if (!connect_active_) {
        connect_active_ = true;
        co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
      }
    }
    co_return;
  }

  std::string object_path_;
  std::string interface_name_;
  std::string mqtt_host_;
  std::string mqtt_port_;
  std::string mqtt_username_;
  std::string mqtt_password_;

  asio::io_context& ctx_;
  ipc_client_type ipc_client_;
  std::shared_ptr<mqtt_client_type> mqtt_client_;
  tfc::logger::logger logger_;
  std::unique_ptr<sdbusplus::asio::connection, std::function<void(sdbusplus::asio::connection*)>> dbus_connection_;
  std::unique_ptr<sdbusplus::bus::match::match, std::function<void(sdbusplus::bus::match::match*)>> signal_updates_;
  tfc::confman::config<config> config_;

  std::vector<std::string> banned_signals_;
  std::vector<tfc::ipc_ruler::signal> active_signals_;
  std::vector<std::variant<std::shared_ptr<details::slot<details::type_bool>>,
                           std::shared_ptr<details::slot<details::type_string>>,
                           std::shared_ptr<details::slot<details::type_int>>,
                           std::shared_ptr<details::slot<details::type_uint>>,
                           std::shared_ptr<details::slot<details::type_double>>,
                           std::shared_ptr<details::slot<details::type_json>>>>
      slots_;

  bool connect_active_ = false;
  bool stop_coroutine_ = false;
};
