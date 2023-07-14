// This program reads signals off the ipc-client and publishes changes to the mqtt broker specified in the command line.

#pragma once
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <cassert>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/string_maker.hpp>
#include "config.hpp"

namespace asio = boost::asio;
namespace details = tfc::ipc::details;

// There is probably a better way to template the config_manager, but my attempts have been futile
// template <class ipc_client_type, class mqtt_client_type, class config_type, template <class> class config_manager>
template <class ipc_client_type, class config_type, template <class> class config_manager>
class mqtt_broadcaster {
public:
  // Constructor used for testing, enables injection of config
  // explicit mqtt_broadcaster(asio::io_context& ctx,
  //                           // ipc_client_type& ipc_client,
  //                           std::shared_ptr<mqtt_client_type> mqtt_client,
  //                           config_manager<config_type>& cfg)
  //     : object_path_(tfc::dbus::make_dbus_path("ipc_ruler")), interface_name_(tfc::dbus::make_dbus_name("manager")),
  //       ctx_(ctx),

  //       ipc_client_(ctx_),

  //       mqtt_client_(std::move(mqtt_client)), logger_("mqtt_broadcaster"), config_(std::move(cfg)) {
  //   properties_callback_ = ipc_client_.register_properties_change_callback(
  //       [this]([[maybe_unused]] sdbusplus::message_t& msg) { add_new_signals(); });
  //   asio::co_spawn(mqtt_client_->strand(), initialize(), asio::detached);
  // }

  // Normal constructor
  mqtt_broadcaster(asio::io_context& io_ctx)

      : object_path_(tfc::dbus::make_dbus_path("ipc_ruler")), interface_name_(tfc::dbus::make_dbus_name("manager")),
        io_ctx_(io_ctx), tls_ctx_{ async_mqtt::tls::context::tlsv12 }, ipc_client_{ io_ctx_ },
        mqtt_client_(std::make_shared<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>>(
            async_mqtt::protocol_version::v5,
            io_ctx.get_executor(),
            tls_ctx_)),
        logger_("mqtt_broadcaster"), config_(io_ctx, "mqtt_broadcaster") {
    properties_callback_ = ipc_client_.register_properties_change_callback(
        [this]([[maybe_unused]] sdbusplus::message_t& msg) { add_new_signals(); });
    asio::co_spawn(mqtt_client_->strand(), initialize(), asio::detached);
  }

private:
  auto initialize() -> asio::awaitable<void> {
    // Initial connect to the broker
    logger_.trace("Initial connect to MQTT broker");

    co_await asio::post(mqtt_client_->strand(), asio::use_awaitable);
    co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);

    // When a new list of allowed string arrives the program is restarted
    // TODO: not sure if parameters are needed
    config_.value()._allowed_topics.observe(
        [this]([[maybe_unused]] auto& new_conf, [[maybe_unused]] auto& old_conf) { add_new_signals(); });

    // When a new variable is set in regard to the mqtt broker the program is restarted
    config_.value()._mqtt_broker_address.observe(
        [this]([[maybe_unused]] auto& _new, [[maybe_unused]] auto& _old) -> asio::awaitable<void> {
          co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
        });

    config_.value()._mqtt_broker_port.observe(
        [this]([[maybe_unused]] auto& _new, [[maybe_unused]] auto& _old) -> asio::awaitable<void> {
          co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
        });

    config_.value()._mqtt_broker_username.observe(
        [this]([[maybe_unused]] auto& _new, [[maybe_unused]] auto& _old) -> asio::awaitable<void> {
          co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
        });

    config_.value()._mqtt_broker_password.observe(
        [this]([[maybe_unused]] auto& _new, [[maybe_unused]] auto& _old) -> asio::awaitable<void> {
          co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
        });

    add_new_signals();
  }

  // Connect the mqtt client to the broker. This function can only be spawned once to avoid multiple spawns to open/close
  // the socket at the same time. The routine that calls this function must set the connect_active_ flag to true before
  // calling this function and the function sets it to false when finished.
  auto connect_to_broker() -> asio::awaitable<void> {
    while (true) {
      try {
        logger_.trace("Connecting to MQTT broker");
        asio::ip::tcp::socket resolve_sock{ io_ctx_ };
        asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
        tls_ctx_.set_default_verify_paths();

        // verify_none : no verification of server certificate
        // verify_peer : verify server certificate, doesn't connect if verification fails

        // verify_fail_if_no_peer_cert : server side
        // verify_client_once : server side, verify the client

        tls_ctx_.set_verify_mode(async_mqtt::tls::verify_peer);
        // If you want to check server certificate, set cacert as follows.
        // tls_ctx_.load_verify_file("/home/magni-the-developer/ssl/ca-root-cert.crt");

        logger_.trace("Resolving IP number");
        asio::ip::tcp::resolver::results_type resolved_ip = co_await res.async_resolve(
            config_.value()._mqtt_broker_address.value(), config_.value()._mqtt_broker_port.value(), asio::use_awaitable);

        logger_.trace("Resovled: {}", resolved_ip.begin()->endpoint().address().to_string());

        logger_.trace("Connecting MQTT client socket to IP number of broker: {}",
                      config_.value()._mqtt_broker_address.value());

        co_await asio::async_connect(mqtt_client_->next_layer().next_layer(), resolved_ip, asio::use_awaitable);

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(mqtt_client_->next_layer().native_handle(),
                                      config_.value()._mqtt_broker_address.value().c_str())) {
          boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
          throw boost::system::system_error{ ec };
        }

        co_await mqtt_client_->next_layer().async_handshake(async_mqtt::tls::stream_base::client, asio::use_awaitable);

        logger_.trace(
            "Sending Connect request to the MQTT broker, clean session is false, keep alive is 100 seconds, "
            "client id is cid1, username is '{}', password is '{}', session expiry interval is UINT_MAX (136 years), domain "
            "name is : {} and port number is : {}",
            config_.value()._mqtt_broker_username.value(), config_.value()._mqtt_broker_password.value(),
            config_.value()._mqtt_broker_address.value(), config_.value()._mqtt_broker_port.value());

        // Connect is initialized with clean start set to false, which means that the broker will keep the session alive
        // even if the client disconnects, and it will keep that session alive for UINT_MAX seconds.
        auto connect_packet = async_mqtt::v5::connect_packet{
          false,
          std::chrono::seconds(100).count(),
          async_mqtt::allocate_buffer("something"),
          async_mqtt::nullopt,
          async_mqtt::allocate_buffer(config_.value()._mqtt_broker_username.value()),
          async_mqtt::allocate_buffer(config_.value()._mqtt_broker_password.value()),
          { async_mqtt::property::session_expiry_interval{ std::numeric_limits<unsigned int>::max() } }
        };

        co_await mqtt_client_->send(connect_packet, asio::use_awaitable);

        logger_.trace("Waiting for MQTT connection acknowledgement");
        auto connack_received = co_await mqtt_client_->recv(
            async_mqtt::filter::match, { async_mqtt::control_packet_type::connack }, asio::use_awaitable);

        // TODO: check for valid connack
        auto connack_packet = connack_received.template get<async_mqtt::v5::connack_packet>();

        std::cout << connack_packet.code() << std::endl;

        // if (connack_packet.code() != async_mqtt::connect_reason_code::success) {
        //   logger_.error("Connection to MQTT broker failed with reason code: {}", connack_packet.code());
        // }

        logger_.trace("Acknowledgement received, connection established");
        co_return;
      } catch (std::exception& e) {
        logger_.error("Error while connecting to MQTT broker: {}", e.what());
      }

      // If the connection failed, then clear the socket and try again after 1 second.
      logger_.trace("Waiting for 1 second and then trying to connect again");
      co_await asio::co_spawn(mqtt_client_->strand(), mqtt_client_->close(asio::use_awaitable), asio::use_awaitable);
      co_await asio::steady_timer{ io_ctx_, std::chrono::seconds{ 10 } }.async_wait(asio::use_awaitable);
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
        active_signals_.emplace_back(signal.name);
      }

      asio::co_spawn(mqtt_client_->strand(), connect_new_slots(new_signals), asio::detached);
    });
  }

  // This function attempts to connect to the signal
  asio::awaitable<bool> attempt_connect_to_signal(std::string signal_name,
                                                  std::shared_ptr<tfc::ipc::details::any_recv> ipc) {
    co_await std::visit(
        [&, this](auto&& receiver) -> asio::awaitable<bool> {
          using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<std::monostate, receiver_t>) {
            auto error = receiver->connect(signal_name);
            if (error) {
              co_return false;
            } else {
              co_return true;
            }
          }
          co_return false;
        },
        *ipc);
  }

  // This function is responsible for receiving and sending the message
  auto receive_and_send_message(std::shared_ptr<tfc::ipc::details::any_recv> ipc) -> asio::awaitable<void> {
    co_await std::visit(
        [&, this](auto&& receiver) -> asio::awaitable<void> {
          using r_t = std::remove_cvref_t<decltype(receiver)>;
          if constexpr (!std::same_as<std::monostate, r_t>) {
            while (true) {
              std::string topic = receiver->_name_();
              std::replace(topic.begin(), topic.end(), '.', '/');

              logger_.trace("Waiting for signal {}", receiver->_name_());
              auto msg = co_await receiver->async_receive(asio::use_awaitable);
              std::string message_value = fmt::format("{}", msg.value());
              if (msg) {
                logger_.trace("Received message: {} from signal {}", message_value, receiver->_name_());
                co_await send_message(topic, message_value);
              } else {
                logger_.error("Failed to receive message: {}", msg.error().message());
              }
            }
          }
        },
        *ipc);
  }

  // This function runs the coroutine for the slot
  auto connect_new_slots(std::vector<std::string> new_slots) -> asio::awaitable<void> {
    for (auto& signal_name : new_slots) {
      auto ipc = std::make_shared<tfc::ipc::details::any_recv>(
          tfc::ipc::details::create_ipc_recv<tfc::ipc::details::any_recv>(io_ctx_, signal_name));
      ipc_receivers_.emplace_back(ipc);
      auto connection = co_await attempt_connect_to_signal(signal_name, ipc);

      if (!connection) {
        logger_.error("Failed to connect complete list of slots {}", signal_name);
      }
    }

    for (std::shared_ptr<tfc::ipc::details::any_recv> receiver : ipc_receivers_) {
      asio::co_spawn(mqtt_client_->strand(), receive_and_send_message(receiver), asio::detached);
    }
  }

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

      co_await asio::post(mqtt_client_->strand(), asio::use_awaitable);
      co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
    }
    co_return;
  }

  std::string object_path_;
  std::string interface_name_;

  asio::io_context& io_ctx_;

  async_mqtt::tls::context tls_ctx_;

  ipc_client_type ipc_client_;

  std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>> mqtt_client_;

  tfc::logger::logger logger_;

  config_manager<config_type> config_;

  std::vector<tfc::ipc::details::any_recv> slots_;

  std::vector<std::string> active_signals_;

  std::vector<std::shared_ptr<tfc::ipc::details::any_recv>> ipc_receivers_;

  std::unique_ptr<sdbusplus::bus::match::match> properties_callback_;
};

// mosquitto_pub -h 3d12e7ab650741769548b74acfce5ec7.s2.eu.hivemq.cloud -p 8883 -u hivemq-magni -P hivemq-magni -t test_topic
// -m "Hello, world"
