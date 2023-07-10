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
    properties_callback_ =
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
    properties_callback_ =
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
        [this]([[maybe_unused]] auto& new_conf, [[maybe_unused]] auto& old_conf) { add_new_signals(); });

    asio::co_spawn(mqtt_client_->strand(), tfc::base::exit_signals(ctx_), asio::detached);

    add_new_signals();
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

  // This function filter out signals that are not allowed using a lazy iterator
  auto clean_signals(const std::vector<tfc::ipc_ruler::signal>& signals) -> std::vector<tfc::ipc_ruler::signal> {
    auto iterator = signals | std::views::filter([this](const tfc::ipc_ruler::signal& signal) {
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

    return new_signals;
  }

  // Function which has oversight over the signals
  auto add_new_signals() -> void {
    logger_.info("Adding new signals");

    ipc_client_.signals([this](const std::vector<tfc::ipc_ruler::signal>& signals) {
      auto cleaned_signals = clean_signals(signals);

      auto iterator = cleaned_signals | std::views::filter([this](const tfc::ipc_ruler::signal& signal) {
                        for (const auto& active_signal : active_signals_) {
                          if (signal.name == active_signal) {
                            return false;
                          }
                        }
                        return true;
                      });

      std::vector<std::string> new_signals;
      for (auto& signal : iterator) {
        logger_.info("Adding signal {}", signal.name);
        new_signals.emplace_back(signal.name);
      }
      // asio::co_spawn(mqtt_client_->strand(), intermediate(new_signals), asio::detached);
      intermediate(new_signals);
    });
  }

  auto intermediate(std::vector<std::string> new_sig) -> void {
    for (auto& new_signal : new_sig) {
      std::cout << new_signal << std::endl;
      // auto& sig = active_signals_.back();
      // TODO: if more than one signal runs this then a segmentation fault occurs
      // if this uses a asio::use_awaitable then it will run one signal and work
      // if it runs asio::detached it will run all signals and not work
      asio::co_spawn(mqtt_client_->strand(), run_slot(new_signal), asio::detached);
      // asio::co_spawn(ctx_, run_slot(new_signal), asio::detached);
    }
  }

  // This function runs the coroutine for the slot
  auto run_slot(std::string signal_name) -> asio::awaitable<void> {
    logger_.info("Running slot for signal {}", signal_name);

    auto ipc{ tfc::ipc::details::create_ipc_recv<tfc::ipc::details::any_recv>(ctx_, signal_name) };
    co_await std::visit(
        [&, this](auto&& receiver) -> asio::awaitable<void> {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<std::monostate, receiver_t>) {
            logger_.trace("Connecting to signal {}", signal_name);
            auto error = receiver->connect(signal_name);
            if (error) {
              logger_.log<tfc::logger::lvl_e::error>("Error: {}", error.message());
            } else {
              co_await std::visit(
                  [&, this](auto&& receiver) -> asio::awaitable<void> {
                    using r_t = std::remove_cvref_t<decltype(receiver)>;
                    if constexpr (!std::same_as<std::monostate, r_t>) {
                      while (true) {
                        logger_.trace("Waiting for signal {}", signal_name);

                        auto msg = co_await ipc.async_receive(asio::use_awaitable);

                        std::string message_value = fmt::format("{}", msg.value());

                        if (msg) {
                          std::cout << " sending on topic " << signal_name << " value " << message_value << std::endl;
                          co_await send_message(signal_name, message_value);
                        } else {
                          logger_.error("Failed to receive message: {}", msg.error().message());
                        }
                      }
                    }
                  },
                  ipc);
            }
          }
        },
        ipc);

    //     slots_.emplace_back([&, this](std::string_view signal_name) -> tfc::ipc::details::any_recv {
    //       auto ipc{ tfc::ipc::details::create_ipc_recv<tfc::ipc::details::any_recv>(ctx_, signal_name) };
    //       co_await std::visit(
    //           [&, this](auto&& receiver) -> asio::awaitable<void> {
    //             using receiver_t = std::remove_cvref_t<decltype(receiver)>;
    //             if constexpr (!std::same_as<std::monostate, receiver_t>) {
    //               logger_.log<tfc::logger::lvl_e::trace>("Connecting to signal {}", signal_name);
    //               auto error = receiver->connect(signal_name);
    //               if (error) {
    //                 logger_.log<tfc::logger::lvl_e::error>("Error: {}", error.message());
    //               } else {
    //                 co_await std::visit(
    //                     [&, this](auto&& slot_) -> asio::awaitable<void> {
    //                       // using receiver_t = std::remove_cvref_t<decltype(slot_)>;
    //                       // if constexpr (!std::same_as<std::monostate, receiver_t>) {
    //                       while (true) {
    //                         logger_.trace("Waiting for signal {}", signal_name);
    //
    //                         auto msg = co_await slot_->async_receive(asio::use_awaitable);
    //
    //                         std::string message_value = fmt::format("{}", msg.value());
    //
    //                         if (msg) {
    //                           std::cout << " sending on topic " << signal_name << " value " << message_value <<
    //                           std::endl; co_await send_message(signal_name, message_value);
    //                         } else {
    //                           logger_.error("Failed to receive message: {}", msg.error().message());
    //                         }
    //                         // }
    //                       }
    //                     },
    //                     ipc);
    //               }
    //             }
    //           },
    //           ipc);
    //       // return ipc
    //           co_return;
    //           ;
    //     }(signal_name));

    // auto& slot = slots_.back();

    // co_await std::visit(
    //     [&, this](auto&& slot_) -> asio::awaitable<void> {
    //       using receiver_t = std::remove_cvref_t<decltype(slot_)>;
    //       if constexpr (!std::same_as<std::monostate, receiver_t>) {
    //         while (true) {
    //           logger_.trace("Waiting for signal {}", signal_name);

    //           auto msg = co_await slot_->async_receive(asio::use_awaitable);

    //           std::string message_value = fmt::format("{}", msg.value());

    //           if (msg) {
    //             std::cout << " sending on topic " << signal_name << " value " << message_value << std::endl;
    //             co_await send_message(signal_name, message_value);
    //           } else {
    //             logger_.error("Failed to receive message: {}", msg.error().message());
    //           }
    //         }
    //       }
    //     },
    //     slot);

    co_return;
  }

  // This function is called when a signal is received
  void update_signals([[maybe_unused]] sdbusplus::message_t& msg) { add_new_signals(); }

  // This function sends a message to the MQTT broker. If the send fails then it will try to restart the broker
  auto send_message(std::string topic, std::string payload) -> asio::awaitable<void> {
    logger_.trace("Sending message {} to topic {}", payload, topic);
    co_await asio::post(mqtt_client_->strand(), asio::use_awaitable);

    bool send_failed = false;
    try {
      auto result = co_await mqtt_client_->send(
          async_mqtt::v5::publish_packet{ mqtt_client_->acquire_unique_packet_id().value(),
                                          async_mqtt::allocate_buffer(topic), async_mqtt::allocate_buffer(payload),
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

  std::vector<tfc::ipc::details::any_recv> slots_;

  // std::vector<tfc::ipc_ruler::signal> active_signals_;
  std::vector<std::string> active_signals_;

  std::unique_ptr<sdbusplus::bus::match::match> properties_callback_;

  bool connect_active_ = false;
};
