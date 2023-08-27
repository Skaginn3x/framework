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

#include <openssl/ssl.h>
#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <tfc/dbus/string_maker.hpp>

#include "config.hpp"
#include "error_codes.hpp"

namespace tfc {

namespace asio = boost::asio;
namespace details = tfc::ipc::details;

using org::eclipse::tahu::protobuf::Payload;
using org::eclipse::tahu::protobuf::Payload_Metric;

class network_manager {
public:
  static auto connect_socket(auto&& socket, auto&& resolved_ip) -> asio::awaitable<void> {
    co_await asio::async_connect(std::forward<decltype(socket)>(socket), std::forward<decltype(resolved_ip)>(resolved_ip),
                                 asio::use_awaitable);
  }

  static auto set_sni_hostname(auto&& native_handle, std::string const& broker_address) -> void {
    if (!SSL_set_tlsext_host_name(std::forward<decltype(native_handle)>(native_handle), broker_address.c_str())) {
      const boost::system::error_code error{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
      throw boost::system::system_error{ error };
    }
  }

  static auto async_handshake(auto&& socket) -> asio::awaitable<void> {
    co_await std::forward<decltype(socket)>(socket).async_handshake(async_mqtt::tls::stream_base::client,
                                                                    asio::use_awaitable);
  }
};

struct signal_data {
  tfc::ipc_ruler::signal information;
  tfc::ipc::details::any_recv receiver;
  std::optional<std::any> current_value;
};

struct scada_signal {
  tfc::ipc::any_signal signal;
  std::string name;
  tfc::ipc::details::type_e type;
};

template <class ipc_client_type, class mqtt_client_type, class config_type, class network_manager_type>
class mqtt_broadcaster {
public:
  static constexpr std::string_view namespace_element = "spBv1.0";
  static constexpr std::string_view ndata = "NDATA";
  static constexpr std::string_view nbirth = "NBIRTH";
  static constexpr std::string_view ndeath = "NDEATH";
  static constexpr std::string_view ncmd = "NCMD";
  static constexpr std::string_view rebirth_metric = "Node Control/Rebirth";

  explicit mqtt_broadcaster(asio::io_context& io_ctx)
      : io_ctx_(io_ctx), tls_ctx_{ async_mqtt::tls::context::tlsv12 }, ipc_client_{ io_ctx_ },
        mqtt_client_(std::make_shared<mqtt_client_type>(async_mqtt::protocol_version::v5, io_ctx.get_executor(), tls_ctx_)),
        logger_("mqtt_broadcaster"), config_(io_ctx, "mqtt_broadcaster"), network_manager_(), timer_{ io_ctx_ } {}

  auto run() -> void {
    properties_callback_ = ipc_client_.register_properties_change_callback(
        [this]([[maybe_unused]] sdbusplus::message_t& msg) { add_new_signals(); });
    asio::co_spawn(mqtt_client_->strand(), initialize(), asio::detached);
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
      if (sig.name != "") {
        switch (sig.type) {
          case tfc::ipc::details::type_e::_bool: {
            scada_signals.emplace_back(tfc::ipc::bool_signal{ io_ctx_, ipc_client_, sig.name, "" },
                                       "mqtt-broadcaster/def/bool/" + sig.name, sig.type);
            break;
          }
          case tfc::ipc::details::type_e::_double_t: {
            scada_signals.emplace_back(tfc::ipc::double_signal{ io_ctx_, ipc_client_, sig.name, "" },
                                       "mqtt-broadcaster/def/double/" + sig.name, sig.type);
            break;
          }
          case tfc::ipc::details::type_e::_int64_t: {
            scada_signals.emplace_back(tfc::ipc::int_signal{ io_ctx_, ipc_client_, sig.name, "" },
                                       "mqtt-broadcaster/def/int64_t/" + sig.name, sig.type);
            break;
          }
          case tfc::ipc::details::type_e::_json: {
            scada_signals.emplace_back(tfc::ipc::json_signal{ io_ctx_, ipc_client_, sig.name, "" },
                                       "mqtt-broadcaster/def/json/" + sig.name, sig.type);
            break;
          }
          case tfc::ipc::details::type_e::_string: {
            scada_signals.emplace_back(tfc::ipc::string_signal{ io_ctx_, ipc_client_, sig.name, "" },
                                       "mqtt-broadcaster/def/string/" + sig.name, sig.type);
            break;
          }
          case tfc::ipc::details::type_e::_uint64_t: {
            scada_signals.emplace_back(tfc::ipc::uint_signal{ io_ctx_, ipc_client_, sig.name, "" },
                                       "mqtt-broadcaster/def/uint64_t/" + sig.name, sig.type);
            break;
          }
          default: {
            logger_.error("Unknown type for signal: {}", sig.name);
            std::exit(-1);
          }
        }
      }
    }
  }

  static auto timestamp_milliseconds() -> std::chrono::milliseconds {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::system_clock;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  }

  auto make_payload() -> Payload {
    Payload payload;
    payload.set_timestamp(timestamp_milliseconds().count());
    if (seq_ == 255) {
      seq_ = 0;
    } else {
      seq_++;
    }

    payload.set_seq(seq_);

    return payload;
  }

  /// This function converts tfc types to Spark Plug B types
  /// More information can be found (page 76) in the spec under section 6.4.16 data types:
  /// https://sparkplug.eclipse.org/specification/version/3.0/documents/sparkplug-specification-3.0.0.pdf
  auto type_enum_convert(tfc::ipc::details::type_e type) -> uint32_t {
    switch (type) {
      case details::type_e::unknown:
        return 0;
      case tfc::ipc::details::type_e::_bool:
        return 11;
      case details::type_e::_int64_t:
        return 4;
      case details::type_e::_uint64_t:
        return 8;
      case details::type_e::_double_t:
        return 10;
      case details::type_e::_string:
      case details::type_e::_json:
        return 12;
    }
    return 0;
  }

  auto format_signal_name(std::string signal_name_to_format) -> std::string {
    std::replace(signal_name_to_format.begin(), signal_name_to_format.end(), '.', '/');
    return signal_name_to_format;
  }

  auto topic_formatter(const std::vector<std::string_view>& topic_vector) -> std::string {
    if (topic_vector.empty()) {
      throw std::runtime_error("Topic can not be empty");
    }
    std::string topic;
    for (const auto& sub_topic : topic_vector) {
      topic += std::string(sub_topic) + "/";
    }
    topic.pop_back();
    return topic;
  }

  auto set_value_payload(Payload_Metric* metric, std::any value) -> void {
    if (value.type() == typeid(bool)) {
      metric->set_boolean_value(std::any_cast<bool>(value));
    } else if (value.type() == typeid(std::string)) {
      metric->set_string_value(std::any_cast<std::string>(value));
    } else if (value.type() == typeid(uint64_t)) {
      metric->set_long_value(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(int64_t)) {
      metric->set_long_value(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(double)) {
      metric->set_double_value(std::any_cast<double>(value));
    } else if (value.type() == typeid(float)) {
      metric->set_float_value(std::any_cast<float>(value));
    } else if (value.type() == typeid(uint32_t)) {
      metric->set_int_value(std::any_cast<uint32_t>(value));
    } else {
      throw std::runtime_error("Unexpected type in std::any.");
    }
  }

  auto set_value_payload(Payload_Metric* metric, const bool& value) -> void { metric->set_boolean_value(value); }

  auto set_value_payload(Payload_Metric* metric, const std::string& value) -> void { metric->set_string_value(value); }

  auto set_value_payload(Payload_Metric* metric, const uint64_t& value) -> void { metric->set_long_value(value); }

  auto set_value_payload(Payload_Metric* metric, const int64_t& value) -> void { metric->set_long_value(value); }

  auto set_value_payload(Payload_Metric* metric, const double& value) -> void { metric->set_double_value(value); }

  auto set_value_payload(Payload_Metric* metric, const float& value) -> void { metric->set_float_value(value); }

  auto set_value_payload(Payload_Metric* metric, const uint32_t& value) -> void { metric->set_int_value(value); }

  auto resolve() -> asio::awaitable<asio::ip::tcp::resolver::results_type> {
    logger_.trace("Resolving the MQTT broker address...");

    asio::ip::tcp::socket resolve_sock{ io_ctx_ };
    asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
    tls_ctx_.set_default_verify_paths();
    tls_ctx_.set_verify_mode(async_mqtt::tls::verify_peer);

    asio::ip::tcp::resolver::results_type resolved_ip =
        co_await res.async_resolve(config_.value().address, std::to_string(config_.value().port), asio::use_awaitable);

    co_return resolved_ip;
  }

  // This function is used to connect to the MQTT broker and perform the handshake
  // If the connection is successful it will return true, otherwise it will return false
  auto connect_and_handshake(asio::ip::tcp::resolver::results_type resolved_ip) -> asio::awaitable<bool> {
    logger_.trace("Resolved the MQTT broker address. Connecting...");

    co_await network_manager_.connect_socket(mqtt_client_->lowest_layer(), resolved_ip);

    logger_.trace("Setting SSL SNI");

    network_manager_.set_sni_hostname(mqtt_client_->next_layer().native_handle(), config_.value().address);

    logger_.trace("Starting SSL handshake");

    co_await network_manager_.async_handshake(mqtt_client_->next_layer());

    auto will_topic = topic_formatter({ namespace_element, config_.value().group_id, ndeath, config_.value().node_id });

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

    logger_.trace("Sending MQTT connection packet...");

    auto send_error = co_await mqtt_client_->send(connect_packet, asio::use_awaitable);

    if (send_error) {
      logger_.error("Error sending MQTT connection packet: {}", send_error.message());
      co_return false;
    }

    logger_.trace("MQTT connection packet sent. Waiting for CONNACK...");

    auto connack_received = co_await mqtt_client_->recv(async_mqtt::filter::match,
                                                        { async_mqtt::control_packet_type::connack }, asio::use_awaitable);

    auto connack_packet = connack_received.template get<async_mqtt::v5::connack_packet>();

    if (connack_packet.code() != async_mqtt::connect_reason_code::success) {
      co_return false;
    }

    logger_.trace("Received CONNACK. Connection successful.");

    co_return true;
  }

  auto connect_to_broker() -> asio::awaitable<void> {
    logger_.trace("Connecting to the MQTT broker...");
    asio::ip::tcp::resolver::results_type resolved_ip = co_await resolve();

    while (true) {
      auto connection_successful = co_await connect_and_handshake(resolved_ip);

      if (connection_successful) {
        co_return;
      }

      co_await asio::co_spawn(mqtt_client_->strand(), mqtt_client_->close(asio::use_awaitable), asio::use_awaitable);
      co_await asio::steady_timer{ io_ctx_, std::chrono::seconds{ 10 } }.async_wait(asio::use_awaitable);
    }
  }

  auto ncmd_listener() -> asio::awaitable<void> {
    co_await secure_subscription();
    while (true) {
      if (auto error = co_await process_ncmd_packet()) {
        logger_.error("Error processing NCMD packet: {}", error.message());
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
    logger_.trace("Starting NCMD listener...");

    std::string ncmd_topic = topic_formatter({ namespace_element, config_.value().group_id, ncmd, config_.value().node_id });

    auto p_id = mqtt_client_->acquire_unique_packet_id();

    logger_.trace("Preparing subscription packet for topic: {}", ncmd_topic);

    auto subscribe_packet = async_mqtt::v5::subscribe_packet{
      *p_id, { { async_mqtt::allocate_buffer(ncmd_topic), async_mqtt::qos::at_most_once | async_mqtt::sub::nl::yes } }
    };

    logger_.trace("Sending subscription packet...");

    co_await mqtt_client_->send(subscribe_packet, asio::use_awaitable);

    logger_.trace("Subscription packet sent. Waiting for SUBACK...");

    auto suback_received = co_await mqtt_client_->recv(async_mqtt::filter::match,
                                                       { async_mqtt::control_packet_type::suback }, asio::use_awaitable);

    logger_.trace("SUBACK received. Checking for errors...");

    auto suback_packet = suback_received.template get<async_mqtt::v5::suback_packet>();

    for (auto const& entry : suback_packet.entries()) {
      if (entry != async_mqtt::suback_reason_code::granted_qos_0) {
        logger_.error("Error subscribing to topic: {}, reason code: {}", ncmd_topic,
                      async_mqtt::suback_reason_code_to_str(entry));
        co_return make_error_code(entry);
      }
    }

    co_return std::error_code{};
  }

  auto process_ncmd_packet() -> asio::awaitable<std::error_code> {
    logger_.trace("Waiting for NCMD packet...");

    auto publish_packet_received = co_await mqtt_client_->recv(
        async_mqtt::filter::match, { async_mqtt::control_packet_type::publish }, asio::use_awaitable);

    auto publish_packet = publish_packet_received.template get_if<async_mqtt::v5::publish_packet>();

    if (!publish_packet) {
      logger_.error("Received packet is not a PUBLISH packet");
      co_return std::error_code{ static_cast<int>(74), std::generic_category() };
    }

    logger_.trace("Received PUBLISH packet. Parsing payload...");

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
      logger_.warn("Received invalid payload. Continuing to next iteration...");
      co_return;
    }

    logger_.trace("Payload parsed successfully. Processing payload...");

    auto metric = payload.metrics(0);

    if (payload.has_timestamp() && !payload.has_seq() &&
        (publish_packet.opts().get_qos() == async_mqtt::qos::at_most_once) &&
        (publish_packet.opts().get_retain() == async_mqtt::pub::retain::no)) {
      if (metric.name() == rebirth_metric) {
        logger_.trace("Conditions met. Sending NBIRTH...");
        co_await asio::co_spawn(mqtt_client_->strand(), send_nbirth(), asio::use_awaitable);
      } else {
        logger_.trace("Metric received. Checking conditions...");
        payload.PrintDebugString();

        if (metric.has_boolean_value()) {
          send_value_on_signal(metric.name(), metric.boolean_value());
        } else if (metric.has_double_value()) {
          send_value_on_signal(metric.name(), metric.double_value());
        } else if (metric.has_float_value()) {
          send_value_on_signal(metric.name(), metric.float_value());
        } else if (metric.has_int_value()) {
          send_value_on_signal(metric.name(), metric.int_value());
        } else if (metric.has_long_value()) {
          send_value_on_signal(metric.name(), metric.long_value());
        } else if (metric.has_string_value()) {
          send_value_on_signal(metric.name(), metric.string_value());
        }
      }
    } else {
      logger_.trace(
          "Conditions not met, payload has timestamp (should be true): {}, "
          "has seq (should be false): {}, qos (should be at_most_once): {}, "
          "retain (should be no): {}, "
          "metric name (should be 'Node Control/Rebirth'): {}",
          payload.has_timestamp(), payload.has_seq(), async_mqtt::qos_to_str(publish_packet.opts().get_qos()),
          async_mqtt::pub::retain_to_str(publish_packet.opts().get_retain()), metric.name());
    }
  }

  auto send_value_on_signal(std::string signal_name,
                            std::variant<bool, double, std::string, int64_t, uint64_t, uint32_t> value) -> void {
    for (auto& sig : scada_signals) {
      if (sig.name == signal_name) {
        std::visit(
            [&value](auto&& signal) -> void {
              using signal_t = std::remove_cvref_t<decltype(signal)>;
              if constexpr (std::is_same_v<signal_t, tfc::ipc::bool_signal>) {
                if (std::holds_alternative<bool>(value)) {
                  signal.send(std::get<bool>(value));
                }
              } else if constexpr (std::is_same_v<signal_t, tfc::ipc::double_signal>) {
                if (std::holds_alternative<double>(value)) {
                  signal.send(std::get<double>(value));
                }
              } else if constexpr (std::is_same_v<signal_t, tfc::ipc::string_signal>) {
                if (std::holds_alternative<std::string>(value)) {
                  signal.send(std::get<std::string>(value));
                }
                // Spark Plug B treats int as uint64_t
              } else if constexpr (std::is_same_v<signal_t, tfc::ipc::int_signal>) {
                if (std::holds_alternative<uint64_t>(value)) {
                  signal.send(std::get<uint64_t>(value));
                }
              } else if constexpr (std::is_same_v<signal_t, tfc::ipc::uint_signal>) {
                if (std::holds_alternative<uint64_t>(value)) {
                  signal.send(std::get<uint64_t>(value));
                }
              } else if constexpr (std::is_same_v<signal_t, tfc::ipc::json_signal>) {
                if (std::holds_alternative<std::string>(value)) {
                  signal.send(std::get<std::string>(value));
                }
              }
            },
            sig.signal);
      }
    }
  }

  auto add_new_signals() -> void {
    logger_.trace("Starting to add new signals...");

    ipc_client_.signals([this](const std::vector<tfc::ipc_ruler::signal>& signals) -> void {
      logger_.trace("Received {} new signals to add.", signals.size());

      signals_.clear();
      signals_.reserve(signals.size());
      for (auto signal : signals) {
        // slot must include type name
        std::string slot_name;
        switch (signal.type) {
          case details::type_e::_bool:
            slot_name = fmt::format("bool_slot_mqtt_broadcaster_{}", signal.name);
            break;
          case details::type_e::unknown:
            slot_name = fmt::format("unknown_slot_mqtt_broadcaster_{}", signal.name);
            break;
          case details::type_e::_int64_t:
            slot_name = fmt::format("int64_t_slot_mqtt_broadcaster_{}", signal.name);
            break;
          case details::type_e::_uint64_t:
            slot_name = fmt::format("uint64_t_slot_mqtt_broadcaster_{}", signal.name);
            break;
          case details::type_e::_double_t:
            slot_name = fmt::format("double_t_slot_mqtt_broadcaster_{}", signal.name);
            break;
          case details::type_e::_string:
            slot_name = fmt::format("string_slot_mqtt_broadcaster_{}", signal.name);
            break;
          case details::type_e::_json:
            slot_name = fmt::format("json_slot_mqtt_broadcaster_{}", signal.name);
            break;
        }

        auto ipc = tfc::ipc::details::create_ipc_recv<tfc::ipc::details::any_recv>(io_ctx_, slot_name);

        std::visit(
            [&](auto&& receiver) -> void {
              using receiver_t = std::remove_cvref_t<decltype(receiver)>;
              if constexpr (!std::same_as<receiver_t, std::monostate>) {
                auto error_code = receiver->connect(signal.name);
                if (error_code) {
                  logger_.trace("Error connecting to signal: {}, error: {}", signal.name, error_code.message());
                }
              }
            },
            ipc);

        signals_.emplace_back(signal, std::move(ipc), std::nullopt);

        logger_.trace("Added signal_data for signal: {}", signal.name);
      }

      logger_.trace("All new signals added. Preparing to send NBIRTH and start signals...");

      // this function is necessary because it is not possible to co_await inside the signals handler
      asio::co_spawn(mqtt_client_->strand(), send_nbirth_and_start_signals(), asio::detached);

      logger_.trace("Sent NBIRTH and started signals.");
    });
  }

  auto send_nbirth_and_start_signals() -> asio::awaitable<void> {
    co_await asio::co_spawn(mqtt_client_->strand(), send_nbirth(), asio::use_awaitable);

    for (auto& signal : signals_) {
      asio::co_spawn(mqtt_client_->strand(), receive_and_send_message(signal), asio::detached);
    }
  }

  auto receive_and_send_message(signal_data& signal_data) -> asio::awaitable<void> {
    logger_.trace("Starting to receive and send messages for signal: {}", signal_data.information.name);

    co_await std::visit(
        [&](auto&& receiver) -> asio::awaitable<void> {
          using r_t = std::remove_cvref_t<decltype(receiver)>;
          std::string topic =
              topic_formatter({ namespace_element, config_.value().group_id, ndata, config_.value().node_id });
          if constexpr (!std::same_as<std::monostate, r_t>) {
            while (true) {
              auto msg = co_await receiver->async_receive(asio::use_awaitable);

              if (msg) {
                logger_.trace("Received a new message for signal: {}", signal_data.information.name);

                signal_data.current_value = msg.value();

                auto payload = make_payload();
                auto* metric = payload.add_metrics();

                if (metric == nullptr) {
                  logger_.error("Failed to add metric to payload for signal: {}", signal_data.information.name);
                  break;
                }

                metric->set_name(format_signal_name(signal_data.information.name));
                metric->set_timestamp(timestamp_milliseconds().count());
                metric->set_datatype(type_enum_convert(signal_data.information.type));

                set_value_payload(metric, msg.value());

                logger_.trace("Payload for signal '{}' prepared.", signal_data.information.name);

                logger_.trace("{}", payload.DebugString());

                std::string payload_string;
                payload.SerializeToString(&payload_string);

                co_await send_message(topic, payload_string, async_mqtt::qos::at_most_once);

                logger_.trace("Sent a message with payload for signal: {}", signal_data.information.name);

              } else {
                logger_.error("Failed to receive message for signal: {}", signal_data.information.name);
                break;
              }
            }
          } else {
            logger_.warn("Receiver is not initialized for signal: {}", signal_data.information.name);
          }
        },
        signal_data.receiver);
  }

  auto send_nbirth() -> asio::awaitable<void> {
    logger_.info("Starting to send nbirth messages");

    std::string topic = topic_formatter({ namespace_element, config_.value().group_id, nbirth, config_.value().node_id });

    Payload payload;
    payload.set_timestamp(timestamp_milliseconds().count());
    seq_ = 0;
    payload.set_seq(seq_);

    auto* node_rebirth = payload.add_metrics();
    node_rebirth->set_name(rebirth_metric.data());
    node_rebirth->set_timestamp(timestamp_milliseconds().count());
    node_rebirth->set_datatype(11);
    node_rebirth->set_boolean_value(false);
    node_rebirth->set_is_transient(false);
    node_rebirth->set_is_historical(false);
    node_rebirth->set_is_null(false);

    logger_.info("Node rebirth metric added to the payload");

    for (signal_data& signal_data : signals_) {
      logger_.info("signals: Processing signal_data: {}", signal_data.information.name);
      auto* variable_metric = payload.add_metrics();
      variable_metric->set_name(format_signal_name(signal_data.information.name));
      variable_metric->set_timestamp(timestamp_milliseconds().count());
      variable_metric->set_datatype(type_enum_convert(signal_data.information.type));

      co_await std::visit(
          [&](auto&& receiver) -> asio::awaitable<void> {
            using receiver_t = std::remove_cvref_t<decltype(receiver)>;
            if constexpr (!std::same_as<std::monostate, receiver_t>) {
              if (signal_data.current_value.has_value()) {
                set_value_payload(variable_metric, signal_data.current_value.value());
              } else {
                logger_.trace("Waiting for initial value for : {}", signal_data.information.name);
                co_await set_initial_value(receiver, variable_metric);
              }
            }
            co_return;
          },
          signal_data.receiver);

      variable_metric->set_is_transient(false);
      variable_metric->set_is_historical(false);

      logger_.info("Signal_data processed and added to the payload: {}", signal_data.information.name);
    }

    payload.PrintDebugString();

    std::string payload_string;
    payload.SerializeToString(&payload_string);

    co_await send_message(topic, payload_string, async_mqtt::qos::at_most_once);

    logger_.info("Nbirth message sent successfully");
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

    asio::co_spawn(mqtt_client_->strand(), async_receive_routine(receiver, read_finished, signal_value),
                   asio::bind_cancellation_slot(cancel_signal_.slot(), boost::asio::detached));

    while (!read_finished) {
      co_await asio::steady_timer{ io_ctx_, std::chrono::milliseconds{ 1 } }.async_wait(asio::use_awaitable);
    }

    if (signal_value != std::nullopt) {
      set_value_payload(variable_metric, signal_value.value());
    } else {
      variable_metric->set_is_null(true);
    }
  }

  auto async_receive_routine(auto&& receiver, bool& read_finished, std::optional<std::any>& signal_value_)
      -> asio::awaitable<void> {
    auto val = co_await receiver->async_receive(asio::use_awaitable);
    if (val.has_value()) {
      signal_value_ = val.value();
    }
    read_finished = true;
  }

  auto timer(std::chrono::steady_clock::duration dur) -> asio::awaitable<void> {
    asio::steady_timer timer(co_await asio::this_coro::executor);
    timer.expires_after(dur);
    co_await timer.async_wait(asio::use_awaitable);
    cancel_signal_.emit(asio::cancellation_type::total);
  }

  auto send_message(std::string topic, std::string payload, async_mqtt::qos _qos) -> asio::awaitable<void> {
    uint16_t packet_id = 0;
    if (_qos != async_mqtt::qos::at_most_once) {
      packet_id = mqtt_client_->acquire_unique_packet_id().value();
    }
    logger_.info("Attempting to send message to topic: {}", topic);

    auto error_code =
        co_await mqtt_client_->send(async_mqtt::v5::publish_packet{ packet_id, async_mqtt::allocate_buffer(topic),
                                                                    async_mqtt::allocate_buffer(payload), _qos },
                                    asio::use_awaitable);

    if (error_code) {
      logger_.error("Failed to send message to topic: {}", topic);
      logger_.info("Attempting to reconnect to broker");
      co_await asio::co_spawn(mqtt_client_->strand(), connect_to_broker(), asio::use_awaitable);
      logger_.info("Reconnected to broker. Attempting to resend the message to topic: {}", topic);
      co_await send_message(topic, payload, _qos);
    } else {
      logger_.info("Message successfully sent to topic: {}", topic);
    }
  }

  asio::io_context& io_ctx_;

  async_mqtt::tls::context tls_ctx_;

  ipc_client_type ipc_client_;

  std::shared_ptr<mqtt_client_type> mqtt_client_;

  tfc::logger::logger logger_;

  config_type config_;

  network_manager_type network_manager_;

  std::vector<signal_data> signals_;

  std::unique_ptr<sdbusplus::bus::match::match> properties_callback_;

  uint64_t seq_ = 0;

  asio::steady_timer timer_;

  asio::cancellation_signal cancel_signal_;

  std::vector<scada_signal> scada_signals;

  friend class testing_mqtt_broadcaster;
};
}  // namespace tfc