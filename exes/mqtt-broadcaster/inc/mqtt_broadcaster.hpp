// This program reads signals off the ipc-client and publishes changes to the mqtt broker specified in the command line.

#pragma once
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/string_maker.hpp>
#include "config.hpp"

namespace asio = boost::asio;
namespace details = tfc::ipc::details;

// There is probably a better way to template the config_manager, but my attempts have been futile
template <class ipc_client_type, class mqtt_client_type, class config_type, template <class> class config_manager>
class mqtt_broadcaster {
public:
  // Constructor used for testing, enables injection of config
  mqtt_broadcaster(asio::io_context& ctx,
                   std::string mqtt_address,
                   std::string mqtt_port,
                   std::string mqtt_username,
                   std::string mqtt_password,
                   ipc_client_type& ipc_client,
                   std::shared_ptr<mqtt_client_type> mqtt_client,
                   config_manager<config_type>& cfg)
      : object_path_(tfc::dbus::make_dbus_path("ipc_ruler")), interface_name_(tfc::dbus::make_dbus_name("manager")),
        mqtt_host_(std::move(mqtt_address)), mqtt_port_(std::move(mqtt_port)), mqtt_username_(std::move(mqtt_username)),
        mqtt_password_(std::move(mqtt_password)), ctx_(ctx), ipc_client_(ipc_client), mqtt_client_(std::move(mqtt_client)),
        logger_("mqtt_broadcaster"), config_(std::move(cfg)) {
    ipc_client.register_properties_change_callback(std::bind_front(&mqtt_broadcaster::update_signals, this));
    asio::co_spawn(mqtt_client_->strand(), initialize(), asio::detached);
  }

  // Normal constructor
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
        logger_("mqtt_broadcaster"), config_(ctx, "mqtt_broadcaster") {
    ipc_client.register_properties_change_callback(std::bind_front(&mqtt_broadcaster::update_signals, this));
    asio::co_spawn(mqtt_client_->strand(), initialize(), asio::detached);
  }

private:
  auto initialize() -> asio::awaitable<void> {
    // Initial connect to the broker
    if (!connect_active_) {
      connect_active_ = true;
      logger_.trace("Initial connect to MQTT broker");
      co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
    }

    // When a new list of allowed string arrives the program is restarted
    config_.value()._allowed_topics.observe(
        [this]([[maybe_unused]] auto& new_conf, [[maybe_unused]] auto& old_conf) { restart(); });

    asio::co_spawn(mqtt_client_->strand(), tfc::base::exit_signals(ctx_), asio::detached);

    load_signals();
  }

  // Connect the mqtt client to the broker. This function can only be spawned once to avoid multiple spawns to open/close
  // the socket at the same time. The routine that calls this function must set the connect_active_ flag to true before
  // calling this function and the function sets it to false when finished.
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

        // Connect is initialized with clean start set to false, which means that the broker will keep the session alive
        // even if the client disconnects, and it will keep that session alive for UINT_MAX seconds.
        auto connect_packet = async_mqtt::v5::connect_packet{ false,
                                                              std::chrono::seconds(100).count(),
                                                              async_mqtt::allocate_buffer("cid1"),
                                                              async_mqtt::nullopt,
                                                              async_mqtt::allocate_buffer(mqtt_username_),
                                                              async_mqtt::allocate_buffer(mqtt_password_),
                                                              { async_mqtt::property::session_expiry_interval{
                                                                  std::numeric_limits<unsigned int>::max() } } };

        co_await mqtt_client_->send(connect_packet, asio::use_awaitable);

        logger_.trace("Waiting for MQTT connection acknowledgement");
        co_await mqtt_client_->recv(async_mqtt::filter::match, { async_mqtt::control_packet_type::connack },
                                    asio::use_awaitable);

        logger_.trace("Acknowledgement received, connection established");
        connect_active_ = false;
        co_return;
      } catch (std::exception& e) {
        logger_.error("Error while connecting to MQTT broker: {}", e.what());
      }

      // If the connection failed, then clear the socket and try again after 1 second.
      logger_.trace("Waiting for 1 second and then trying to connect again");
      co_await asio::co_spawn(mqtt_client_->strand(), mqtt_client_->close(asio::use_awaitable), asio::use_awaitable);
      co_await asio::steady_timer{ ctx_, std::chrono::seconds{ 1 } }.async_wait(asio::use_awaitable);
    }
  }

  // Function which has oversight over the signals
  auto load_signals() -> void {
    logger_.info("Cancelling running slots");


    ipc_client_.signals([this](const std::vector<tfc::ipc_ruler::signal>& signals) {

      // Stop reading the current signals by canceling the slots
      for (auto& slot : slots_) {
        std::cout << "Canceling slot" << slot->name << std::endl;
        // std::visit([](auto& ptr) { ptr->cancel(); }, slot);
      }

      logger_.info("Received {} signals from IPC client", signals.size());
      active_signals_ = signals;
      clean_signals();
      for (tfc::ipc_ruler::signal& signal : active_signals_) {
        handle_signal(signal);
      }
    });
  }

  // This function filter out signals that are not allowed using a lazy iterator
  auto clean_signals() -> void {

    auto iterator = active_signals_ | std::views::filter([this](const tfc::ipc_ruler::signal& signal) {
                      for (const auto& topic : config_.value()._allowed_topics.value()) {
                        if (signal.name.find(topic) != std::string::npos) {
                          return true;
                        }
                      }
                      return false;
                    });

    std::vector<tfc::ipc_ruler::signal> new_signals;
    for (auto& signal : iterator) {
      new_signals.emplace_back(signal);
    }

    active_signals_ = new_signals;

  }

  // This function checks the type of the signal in order to determine how to read from the slot
  auto handle_signal(tfc::ipc_ruler::signal& signal) -> void {
    logger_.info("Handling signal {}", signal.name);
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
    logger_.info("Running slot for signal {}", signal.name);
    slots_.emplace_back(std::make_shared<details::slot<slot_type>>(ctx_, signal.name));
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
  void update_signals([[maybe_unused]] sdbusplus::message_t& msg) { restart(); }

  // This function runs a coroutine for a slot, when a new value is received a message is sent to the MQTT broker
  template <class slot_type, class value_type>
  auto slot_coroutine(details::slot<slot_type>& slot, std::string signal_name) -> asio::awaitable<void> {
    logger_.info("Starting coroutine for signal: {}", signal_name);
    std::replace(signal_name.begin(), signal_name.end(), '.', '/');

    while (!stop_coroutine_) {
      std::expected<value_type, std::error_code> msg = co_await slot.async_receive(asio::use_awaitable);
      if (msg) {
        std::string message_value = fmt::format("{}", msg.value());
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

  config_manager<config_type> config_;

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
