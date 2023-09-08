#pragma once

#include <algorithm>
#include <any>
#include <chrono>
#include <concepts>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <tfc/dbus/string_maker.hpp>

#include <impl/impl.hpp>
#include "config.hpp"
#include "error_codes.hpp"
#include "mqtt_client.hpp"

#include <impl/network_manager_ssl.hpp>
#include <impl/sparkplug_b_spec.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;
namespace details = tfc::ipc::details;

using org::eclipse::tahu::protobuf::Payload;
using org::eclipse::tahu::protobuf::Payload_Metric;

template <class ipc_client_type, class config_type, class network_manager_type>
class broadcaster {
private:
public:
  static constexpr std::string_view namespace_element = "spBv1.0";
  static constexpr std::string_view ndata = "NDATA";
  static constexpr std::string_view nbirth = "NBIRTH";
  static constexpr std::string_view ndeath = "NDEATH";
  static constexpr std::string_view ncmd = "NCMD";
  static constexpr std::string_view rebirth_metric = "Node Control/Rebirth";

  explicit broadcaster(asio::io_context& io_ctx) : io_ctx_(io_ctx) {
    if (config_.value().ssl_active == tfc::mqtt::ssl::no) {
      mqtt_client_ =
          std::make_unique<std::variant<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>,
                                        async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>>>(
              async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>{ async_mqtt::protocol_version::v5,
                                                                                          io_ctx_.get_executor() });
    } else if (config_.value().ssl_active == tfc::mqtt::ssl::yes) {
      mqtt_client_ =
          std::make_unique<std::variant<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>,
                                        async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>>>(
              async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>{
                  async_mqtt::protocol_version::v5, io_ctx_.get_executor(), tls_ctx_ });
    }
  }

  auto get_unique_packet_id() {
    return std::visit([](auto&& arg) { return arg.acquire_unique_packet_id(); }, *mqtt_client_);
  }

  template <typename packet_t>
  auto send_via_variant(packet_t&& packet) {
    return std::visit([&packet](auto&& endpoint) { return endpoint.send(std::move(packet), asio::use_awaitable); },
                      *mqtt_client_);
  }

  auto recv_via_variant(async_mqtt::control_packet_type&& packet_type) {
    return std::visit(
        [&packet_type](auto&& endpoint) {
          return endpoint.recv(async_mqtt::filter::match, { packet_type }, asio::use_awaitable);
        },
        *mqtt_client_);
  }

  auto run() -> void {
    properties_callback_ = ipc_client_.register_properties_change_callback(
        [this]([[maybe_unused]] sdbusplus::message_t& msg) { add_new_signals(); });

    std::visit([this](auto&& arg) { asio::co_spawn(arg.strand(), initialize(), asio::detached); }, *mqtt_client_);

    io_ctx_.run();
  }

private:
  auto initialize() -> asio::awaitable<void> {
    create_scada_signals();
    co_await connect_to_broker();
    add_new_signals();
    co_await ncmd_listener();
  }

  auto create_scada_signals() -> void {
    for (auto const& sig : config_.value().scada_signals) {
      if (!sig.name.empty()) {
        switch (sig.type) {
          using enum tfc::ipc::details::type_e;
          case _bool: {
            scada_signals_.emplace_back(tfc::ipc::bool_signal{ io_ctx_, ipc_client_, sig.name, sig.description });
            break;
          }
          case _double_t: {
            scada_signals_.emplace_back(tfc::ipc::double_signal{ io_ctx_, ipc_client_, sig.name, sig.description });
            break;
          }
          case _int64_t: {
            scada_signals_.emplace_back(tfc::ipc::int_signal{ io_ctx_, ipc_client_, sig.name, sig.description });
            break;
          }
          case _json: {
            scada_signals_.emplace_back(tfc::ipc::json_signal{ io_ctx_, ipc_client_, sig.name, sig.description });
            break;
          }
          case _string: {
            scada_signals_.emplace_back(tfc::ipc::string_signal{ io_ctx_, ipc_client_, sig.name, sig.description });
            break;
          }
          case _uint64_t: {
            scada_signals_.emplace_back(tfc::ipc::uint_signal{ io_ctx_, ipc_client_, sig.name, sig.description });
            break;
          }
          case unknown: {
            outgoing_logger_.error("Unknown type for signal: {}", sig.name);
          }
        }
      }
    }
  }

  auto resolve() -> asio::awaitable<asio::ip::tcp::resolver::results_type> {
    networking_logger_.trace("Resolving the MQTT broker address...");

    asio::ip::tcp::socket resolve_sock{ io_ctx_ };
    asio::ip::tcp::resolver res{ resolve_sock.get_executor() };

    switch (config_.value().ssl_active) {
      case tfc::mqtt::ssl::yes: {
        // tls_ctx_.set_default_verify_paths();
        // tls_ctx_.set_verify_mode(async_mqtt::tls::verify_peer);
        break;
      }
      case tfc::mqtt::ssl::no: {
        break;
      }
    }

    asio::ip::tcp::resolver::results_type resolved_ip =
        co_await res.async_resolve(config_.value().address, impl::port_to_string(config_.value().port), asio::use_awaitable);

    co_return resolved_ip;
  }

  // This function is used to connect to the MQTT broker and perform the handshake
  // If the connection is successful it will return true, otherwise it will return false
  auto connect_and_handshake(asio::ip::tcp::resolver::results_type resolved_ip) -> asio::awaitable<bool> {
    networking_logger_.trace("Resolved the MQTT broker address. Connecting...");

    co_await std::visit(
        [&resolved_ip](auto&& arg) { return asio::async_connect(arg.lowest_layer(), resolved_ip, asio::use_awaitable); },
        *mqtt_client_);

    networking_logger_.trace("Setting SSL SNI");

    std::visit(
        [this](auto&& arg) {
          return network_manager_.set_sni_hostname(arg.next_layer().native_handle(), config_.value().address);
        },
        *mqtt_client_);

    networking_logger_.trace("Starting SSL handshake");

    co_await std::visit([this](auto&& arg) { return network_manager_.async_handshake(arg.next_layer()); }, *mqtt_client_);

    auto will_topic =
        impl::topic_formatter({ namespace_element, config_.value().group_id, ndeath, config_.value().node_id });

    // SparkPlugB spec specifies that clean start must be true and Session Expiry Interval must be 0
    auto connect_packet =
        async_mqtt::v5::connect_packet{ true,
                                        std::chrono::seconds(100).count(),
                                        async_mqtt::allocate_buffer(config_.value().client_id),
                                        async_mqtt::will(async_mqtt::allocate_buffer(will_topic),
                                                         async_mqtt::buffer("DEATH"), async_mqtt::qos::at_least_once),
                                        async_mqtt::allocate_buffer(config_.value().username),
                                        async_mqtt::allocate_buffer(config_.value().password),
                                        { async_mqtt::property::session_expiry_interval{ 0 } } };

    networking_logger_.trace("Sending MQTT connection packet...");

    auto send_error = co_await send_via_variant(std::move(connect_packet));

    if (send_error) {
      networking_logger_.error("Error sending MQTT connection packet: {}", send_error.message());
      co_return false;
    }

    networking_logger_.trace("MQTT connection packet sent. Waiting for CONNACK...");

    auto connack_received = co_await recv_via_variant(
        // async_mqtt::filter::match,
        // {
        async_mqtt::control_packet_type::connack
        // }, asio::use_awaitable
    );

    // Further processing based on connack_received

    // auto connack_received = co_await mqtt_client_->recv(async_mqtt::filter::match, {
    // async_mqtt::control_packet_type::connack }, asio::use_awaitable);

    auto connack_packet = connack_received.template get<async_mqtt::v5::connack_packet>();

    if (connack_packet.code() != async_mqtt::connect_reason_code::success) {
      co_return false;
    }

    networking_logger_.trace("Received CONNACK. Connection successful.");

    co_return true;
  }

  auto connect_to_broker() -> asio::awaitable<void> {
    networking_logger_.trace("Connecting to the MQTT broker...");
    asio::ip::tcp::resolver::results_type resolved_ip = co_await resolve();

    while (true) {
      auto connection_successful = co_await connect_and_handshake(resolved_ip);

      if (connection_successful) {
        co_return;
      }

      co_await std::visit(
          [](auto&& client_) {
            return asio::co_spawn(std::forward<decltype(client_)>(client_).strand(),
                                  std::forward<decltype(client_)>(client_).close(asio::use_awaitable), asio::use_awaitable);
          },
          *mqtt_client_);

      co_await asio::steady_timer{ io_ctx_, std::chrono::seconds{ 10 } }.async_wait(asio::use_awaitable);
    }
  }

  auto ncmd_listener() -> asio::awaitable<void> {
    co_await secure_subscription();
    while (true) {
      if (auto error = co_await process_ncmd_packet()) {
        incoming_logger_.error("Error processing NCMD packet: {}", error.message());
        co_await secure_subscription();
      }
    }
  }

  auto secure_subscription() -> asio::awaitable<void> {
    while (true) {
      auto error_code = co_await subscribe_to_ncmd_topic();
      if (!error_code) {
        break;
      }
      co_await connect_to_broker();
    }
  }

  auto subscribe_to_ncmd_topic() -> asio::awaitable<std::error_code> {
    incoming_logger_.trace("Starting NCMD listener...");

    std::string ncmd_topic =
        impl::topic_formatter({ namespace_element, config_.value().group_id, ncmd, config_.value().node_id });

    incoming_logger_.trace("Sending subscription packet...");

    co_await send_via_variant(async_mqtt::v5::subscribe_packet{
        get_unique_packet_id().value(),
        { { async_mqtt::allocate_buffer(ncmd_topic), async_mqtt::qos::at_most_once | async_mqtt::sub::nl::yes } } });

    incoming_logger_.trace("Subscription packet sent. Waiting for SUBACK...");

    auto suback_received = co_await recv_via_variant(async_mqtt::control_packet_type::suback);

    incoming_logger_.trace("SUBACK received. Checking for errors...");

    auto suback_packet = suback_received.template get<async_mqtt::v5::suback_packet>();

    for (auto const& entry : suback_packet.entries()) {
      if (entry != async_mqtt::suback_reason_code::granted_qos_0) {
        incoming_logger_.error("Error subscribing to topic: {}, reason code: {}", ncmd_topic,
                               async_mqtt::suback_reason_code_to_str(entry));
        co_return make_error_code(entry);
      }
    }

    co_return std::error_code{};
  }

  auto process_ncmd_packet() -> asio::awaitable<std::error_code> {
    incoming_logger_.trace("Waiting for NCMD packet...");

    // auto publish_packet_received = co_await mqtt_client_->recv(
    // async_mqtt::filter::match, { async_mqtt::control_packet_type::publish }, asio::use_awaitable);

    //  auto publish_packet_received = co_await recv_via_variant(
    //      mqtt_client_, async_mqtt::filter::match, { async_mqtt::control_packet_type::publish }, asio::use_awaitable);

    auto publish_packet_received = co_await recv_via_variant(async_mqtt::control_packet_type::publish);

    auto publish_packet = publish_packet_received.template get_if<async_mqtt::v5::publish_packet>();

    if (!publish_packet) {
      incoming_logger_.error("Received packet is not a PUBLISH packet");
      co_return std::error_code{ static_cast<int>(74), std::generic_category() };
    }

    incoming_logger_.trace("Received PUBLISH packet. Parsing payload...");

    for (long unsigned int i = 0; i < publish_packet->payload().size(); i++) {
      auto data = publish_packet->payload()[i];
      co_await process_payload(data, *publish_packet);
    }

    co_return std::error_code{};
  }

  auto process_payload(auto&& data, async_mqtt::v5::publish_packet publish_packet) -> asio::awaitable<void> {
    Payload payload;

    bool payload_valid = payload.ParseFromArray(data.data(), data.size());

    if (!payload_valid) {
      incoming_logger_.warn("Received invalid payload. Continuing to next iteration...");
      co_return;
    }

    incoming_logger_.trace("Payload parsed successfully. Processing payload...");

    auto metric = payload.metrics(0);

    incoming_logger_.trace("Incoming NCMD payload: \n {}", payload.DebugString());

    // strictly speaking, the NCMD packet should not have a seq nr but AVEVA does not follow this rule
    if (payload.has_timestamp() && (publish_packet.opts().get_qos() == async_mqtt::qos::at_most_once) &&
        (publish_packet.opts().get_retain() == async_mqtt::pub::retain::no)) {
      if (metric.name() == rebirth_metric) {
        incoming_logger_.trace("Conditions met. Sending NBIRTH...");
        co_await std::visit([this](auto&& arg) { return asio::co_spawn(arg.strand(), send_nbirth(), asio::use_awaitable); },
                            *mqtt_client_);
      } else {
        incoming_logger_.trace("Metric received. Checking conditions...");

        if (metric.has_boolean_value()) {
          std::cout << "has boolean value\n";
          send_value_on_signal(metric.name(), metric.boolean_value());
        } else if (metric.has_double_value()) {
          std::cout << "has double value\n";
          send_value_on_signal(metric.name(), metric.double_value());
        } else if (metric.has_float_value()) {
          std::cout << "has float value\n";
          send_value_on_signal(metric.name(), metric.float_value());
        } else if (metric.has_int_value()) {
          std::cout << "has int value\n";
          send_value_on_signal(metric.name(), static_cast<uint64_t>(metric.int_value()));
        } else if (metric.has_long_value()) {
          std::cout << "has long value\n";
          send_value_on_signal(metric.name(), metric.long_value());
        } else if (metric.has_string_value()) {
          std::cout << "has string value\n";
          send_value_on_signal(metric.name(), metric.string_value());
        }
      }
    } else {
      incoming_logger_.trace(
          "Conditions not met, payload has timestamp (should be true): {}, "
          "has seq (should be false): {}, qos (should be at_most_once): {}, "
          "retain (should be no): {}, "
          "metric name: {}",
          payload.has_timestamp(), payload.has_seq(), async_mqtt::qos_to_str(publish_packet.opts().get_qos()),
          async_mqtt::pub::retain_to_str(publish_packet.opts().get_retain()), metric.name());
    }
  }

  auto send_value_on_signal(std::string signal_name, std::variant<bool, double, std::string, int64_t, uint64_t> value) {
    for (auto& sig : scada_signals_) {
      std::visit(
          [&value, &signal_name]<typename signal_t>(signal_t& signal) {
            if (signal_name.ends_with(signal.name())) {
              using value_t = typename std::remove_cvref_t<signal_t>::value_t;

              if constexpr (std::is_same_v<value_t, int64_t>) {
                signal.send(static_cast<int64_t>(std::get<uint64_t>(value)));
              } else {
                signal.send(std::get<value_t>(value));
              }
            }
          },
          sig);
    }
  }

  auto add_new_signals() -> void {
    outgoing_logger_.trace("Starting to add new signals...");

    ipc_client_.signals([this](const std::vector<tfc::ipc_ruler::signal>& signals) -> void {
      outgoing_logger_.trace("Received {} new signals to add.", signals.size());

      signals_.clear();
      signals_.reserve(signals.size());
      for (auto signal : signals) {
        // slot must include type name
        tfc::ipc::details::any_recv ipc;
        switch (signal.type) {
          case details::type_e::_bool:
            ipc = tfc::ipc::details::bool_recv::create(io_ctx_, signal.name);
            break;
          case details::type_e::_int64_t:
            ipc = tfc::ipc::details::int_recv::create(io_ctx_, signal.name);
            break;
          case details::type_e::_uint64_t:
            ipc = tfc::ipc::details::uint_recv::create(io_ctx_, signal.name);
            break;
          case details::type_e::_double_t:
            ipc = tfc::ipc::details::double_recv::create(io_ctx_, signal.name);
            break;
          case details::type_e::_string:
            ipc = tfc::ipc::details::string_recv::create(io_ctx_, signal.name);
            break;
          case details::type_e::_json:
            ipc = tfc::ipc::details::json_recv::create(io_ctx_, signal.name);
            break;
          default:
            outgoing_logger_.warn(fmt::format("Unsupported type {}", static_cast<uint16_t>(signal.type)));
        }

        std::visit(
            [&](auto&& receiver) -> void {
              using receiver_t = std::remove_cvref_t<decltype(receiver)>;
              if constexpr (!std::same_as<receiver_t, std::monostate>) {
                auto error_code = receiver->connect(signal.name);
                if (error_code) {
                  outgoing_logger_.trace("Error connecting to signal: {}, error: {}", signal.name, error_code.message());
                }
              }
            },
            ipc);

        signals_.emplace_back(signal, std::move(ipc), std::nullopt);

        outgoing_logger_.trace("Added signal_data for signal: {}", signal.name);
      }

      outgoing_logger_.trace("All new signals added. Preparing to send NBIRTH and start signals...");

      std::visit([this](auto&& arg) { asio::co_spawn(arg.strand(), send_nbirth_and_start_signals(), asio::detached); },
                 *mqtt_client_);

      outgoing_logger_.trace("Sent NBIRTH and started signals.");
    });
  }

  auto send_nbirth_and_start_signals() -> asio::awaitable<void> {
    co_await std::visit([this](auto&& arg) { return asio::co_spawn(arg.strand(), send_nbirth(), asio::use_awaitable); },
                        *mqtt_client_);

    for (auto& signal : signals_) {
      std::visit(
          [this, &signal](auto&& arg) { asio::co_spawn(arg.strand(), receive_and_send_message(signal), asio::detached); },
          *mqtt_client_);
    }
  }

  auto receive_and_send_message(impl::signal_data& signal_data) -> asio::awaitable<void> {
    outgoing_logger_.trace("Starting to receive and send messages for signal: {}", signal_data.information.name);

    co_await std::visit(
        [&](auto&& receiver) -> asio::awaitable<void> {
          using r_t = std::remove_cvref_t<decltype(receiver)>;
          std::string topic =
              impl::topic_formatter({ namespace_element, config_.value().group_id, ndata, config_.value().node_id });
          if constexpr (!std::same_as<std::monostate, r_t>) {
            while (true) {
              auto msg = co_await receiver->async_receive(asio::use_awaitable);

              if (msg) {
                outgoing_logger_.trace("Received a new message for signal: {}", signal_data.information.name);

                signal_data.current_value = msg.value();

                auto payload = impl::make_payload(seq_);
                auto* metric = payload.add_metrics();

                if (metric == nullptr) {
                  outgoing_logger_.error("Failed to add metric to payload for signal: {}", signal_data.information.name);
                  break;
                }

                metric->set_name(impl::format_signal_name(signal_data.information.name));
                metric->set_timestamp(impl::timestamp_milliseconds().count());
                metric->set_datatype(impl::type_enum_convert(signal_data.information.type));

                impl::set_value_payload(metric, msg.value());

                outgoing_logger_.trace("Payload for signal '{}' prepared.", signal_data.information.name);

                outgoing_logger_.trace("Payload: \n {}", payload.DebugString());

                std::string payload_string;
                payload.SerializeToString(&payload_string);

                co_await send_message(topic, payload_string, async_mqtt::qos::at_most_once);

                outgoing_logger_.trace("Sent a message with payload for signal: {}", signal_data.information.name);

              } else {
                outgoing_logger_.error("Failed to receive message for signal: {}", signal_data.information.name);
                break;
              }
            }
          } else {
            outgoing_logger_.warn("Receiver is not initialized for signal: {}", signal_data.information.name);
          }
        },
        signal_data.receiver);
  }

  auto send_nbirth() -> asio::awaitable<void> {
    outgoing_logger_.info("Starting to send nbirth messages");

    std::string topic =
        impl::topic_formatter({ namespace_element, config_.value().group_id, nbirth, config_.value().node_id });

    Payload payload;
    payload.set_timestamp(impl::timestamp_milliseconds().count());
    seq_ = 0;
    payload.set_seq(seq_);

    auto* node_rebirth = payload.add_metrics();
    node_rebirth->set_name(rebirth_metric.data());
    node_rebirth->set_timestamp(impl::timestamp_milliseconds().count());
    node_rebirth->set_datatype(11);
    node_rebirth->set_boolean_value(false);
    node_rebirth->set_is_transient(false);
    node_rebirth->set_is_historical(false);
    node_rebirth->set_is_null(false);

    outgoing_logger_.info("Node rebirth metric added to the payload");

    for (impl::signal_data& signal_data : signals_) {
      outgoing_logger_.info("signals: Processing signal_data: {}", signal_data.information.name);
      auto* variable_metric = payload.add_metrics();
      variable_metric->set_name(impl::format_signal_name(signal_data.information.name));
      variable_metric->set_timestamp(impl::timestamp_milliseconds().count());
      variable_metric->set_datatype(impl::type_enum_convert(signal_data.information.type));

      co_await std::visit(
          [&](auto&& receiver) -> asio::awaitable<void> {
            using receiver_t = std::remove_cvref_t<decltype(receiver)>;
            if constexpr (!std::same_as<std::monostate, receiver_t>) {
              if (signal_data.current_value.has_value()) {
                impl::set_value_payload(variable_metric, signal_data.current_value.value());
              } else {
                outgoing_logger_.trace("Waiting for initial value for : {}", signal_data.information.name);
                co_await set_initial_value(receiver, variable_metric);
              }
            }
            co_return;
          },
          signal_data.receiver);

      variable_metric->set_is_transient(false);
      variable_metric->set_is_historical(false);

      outgoing_logger_.info("Signal_data processed and added to the payload: {}", signal_data.information.name);
    }

    outgoing_logger_.trace("Payload: \n {}", payload.DebugString());

    std::string payload_string;
    payload.SerializeToString(&payload_string);

    co_await send_message(topic, payload_string, async_mqtt::qos::at_most_once);

    outgoing_logger_.info("Nbirth message sent successfully");
  }

  // This function reads the initial value off the signal.
  // The ideal solution is to use awaitable operators, but they are not compatible with azmq for an unknown reason.
  auto set_initial_value(auto&& receiver, Payload_Metric* variable_metric) -> asio::awaitable<void> {
    bool read_finished = false;
    std::optional<std::any> signal_value;

    asio::steady_timer timer(co_await asio::this_coro::executor);
    timer.expires_after(std::chrono::milliseconds(20));

    timer.async_wait([&](boost::system::error_code error_code) {
      if (!error_code) {
        cancel_signal_.emit(asio::cancellation_type::all);
        read_finished = true;
      }
    });

    std::visit(
        [this, &receiver, &read_finished, &signal_value](auto&& arg) {
          asio::co_spawn(arg.strand(), async_receive_routine(receiver, read_finished, signal_value),
                         asio::bind_cancellation_slot(cancel_signal_.slot(), boost::asio::detached));
        },
        *mqtt_client_);

    while (!read_finished) {
      co_await asio::steady_timer{ io_ctx_, std::chrono::milliseconds{ 1 } }.async_wait(asio::use_awaitable);
    }

    if (signal_value != std::nullopt) {
      impl::set_value_payload(variable_metric, signal_value.value());
    } else {
      variable_metric->set_is_null(true);
    }
  }

  auto async_receive_routine(auto&& receiver, bool& read_finished, std::optional<std::any>& signal_value)
      -> asio::awaitable<void> {
    auto val = co_await receiver->async_receive(asio::use_awaitable);
    if (val.has_value()) {
      signal_value = val.value();
    }
    read_finished = true;
  }

  auto timer(std::chrono::steady_clock::duration dur) -> asio::awaitable<void> {
    asio::steady_timer timer(co_await asio::this_coro::executor);
    timer.expires_after(dur);
    co_await timer.async_wait(asio::use_awaitable);
    cancel_signal_.emit(asio::cancellation_type::total);
  }

  auto send_message(std::string topic, std::string payload, async_mqtt::qos qos) -> asio::awaitable<void> {
    uint16_t packet_id = 0;
    if (qos != async_mqtt::qos::at_most_once) {
      packet_id = get_unique_packet_id().value();
    }
    networking_logger_.info("Attempting to send message to topic: {}", topic);

    auto error_code = co_await send_via_variant(async_mqtt::v5::publish_packet{
        packet_id, async_mqtt::allocate_buffer(topic), async_mqtt::allocate_buffer(payload), qos });

    if (error_code) {
      networking_logger_.error("Failed to send message to topic: {}", topic);
      networking_logger_.info("Attempting to reconnect to broker");

      co_await std::visit(
          [this](auto&& arg) { return asio::co_spawn(arg.strand(), connect_to_broker(), asio::use_awaitable); },
          *mqtt_client_);

      networking_logger_.info("Reconnected to broker. Attempting to resend the message to topic: {}", topic);
      co_await send_message(topic, payload, qos);
    } else {
      networking_logger_.info("Message successfully sent to topic: {}", topic);
    }
  }

  asio::io_context& io_ctx_;

  async_mqtt::tls::context tls_ctx_{ async_mqtt::tls::context::tlsv12 };

  config_type config_{ io_ctx_, "mqtt_broadcaster" };

  std::unique_ptr<std::variant<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>,
                               async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>>>
      mqtt_client_;

  ipc_client_type ipc_client_{ io_ctx_ };

  // pertains to all networking communication
  tfc::logger::logger networking_logger_{ "networking" };

  // incoming means from scada to tfc
  tfc::logger::logger incoming_logger_{ "incoming" };

  // outgoing means from tfc to scada
  tfc::logger::logger outgoing_logger_{ "outgoing" };

  network_manager_type network_manager_{};

  std::vector<impl::signal_data> signals_;

  std::unique_ptr<sdbusplus::bus::match::match> properties_callback_;

  uint64_t seq_ = 0;

  asio::steady_timer timer_{ io_ctx_ };

  asio::cancellation_signal cancel_signal_;

  std::vector<tfc::ipc::any_signal> scada_signals_;

  friend class testing_mqtt_broadcaster;
};
}  // namespace tfc::mqtt
