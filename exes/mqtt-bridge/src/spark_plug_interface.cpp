#include <any>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>

#include <constants.hpp>
#include <spark_plug_interface.hpp>
#include <structs.hpp>
#include <tfc/ipc.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

template <class config_t, class mqtt_client_t>
spark_plug_interface<config_t, mqtt_client_t>::spark_plug_interface(asio::io_context& io_ctx)
    : io_ctx_(io_ctx), ncmd_topic_(topic_formatter({ constants::namespace_element, config_.value().group_id, constants::ncmd,
                                                     config_.value().node_id })),
      mqtt_will_topic_(topic_formatter(
          { constants::namespace_element, config_.value().group_id, constants::ndeath, config_.value().node_id })),
      ndata_topic_(topic_formatter(
          { constants::namespace_element, config_.value().group_id, constants::ndata, config_.value().node_id })) {
  const std::string_view mqtt_will_payload{ make_will_payload() };

  mqtt_client_ = std::make_unique<mqtt_client_t>(io_ctx_, mqtt_will_topic_, mqtt_will_payload);
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::get_bd_seq() -> int64_t {
  return bdSeq_++ % 256;
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::make_payload() -> Payload {
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

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::update_value(tfc::ipc::details::any_slot_cb& signal_data) -> void {
  asio::co_spawn(strand(), update_value_impl(signal_data), asio::detached);
};

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::update_value_impl(tfc::ipc::details::any_slot_cb& signal_data)
    -> asio::awaitable<void> {
  std::string variable;
  ipc::details::type_e type;

  std::visit(
      [&variable, &type]<typename recv_t>(recv_t&& receiver) {
        using receiver_t = std::remove_cvref_t<decltype(receiver)>;
        if constexpr (!std::same_as<receiver_t, std::monostate>) {
          variable = format_signal_name(receiver->name().data());
          type = receiver->type().value_e;
        }
      },
      signal_data);

  Payload payload = make_payload();
  Payload_Metric* metric = payload.add_metrics();

  if (metric == nullptr) {
    logger_.error("Failed to add metric to payload for variable: {}", variable);
    co_return;
  }

  metric->set_name(variable);
  metric->set_timestamp(timestamp_milliseconds().count());
  metric->set_datatype(type_enum_convert(type));

  std::visit(
      [this, &metric]<typename recv_t>(recv_t&& receiver) {
        using receiver_t = std::remove_cvref_t<decltype(receiver)>;
        if constexpr (std::same_as<receiver_t, ipc::details::bool_slot_cb_ptr>) {
          set_value_payload(metric, receiver->value().value_or(false), logger_);
        } else if constexpr (std::same_as<receiver_t, ipc::details::int_slot_cb_ptr> ||
                             std::same_as<receiver_t, ipc::details::uint_slot_cb_ptr> ||
                             std::same_as<receiver_t, ipc::details::double_slot_cb_ptr>) {
          set_value_payload(metric, receiver->value().value_or(0), logger_);
        } else if constexpr (std::same_as<receiver_t, ipc::details::string_slot_cb_ptr> ||
                             std::same_as<receiver_t, ipc::details::json_slot_cb_ptr>) {
          set_value_payload(metric, receiver->value().value_or(""), logger_);
        }
      },
      signal_data);

  logger_.trace("Updating variable: {}", variable);
  logger_.trace("Payload: {}", payload.DebugString());

  std::string payload_string;
  payload.SerializeToString(&payload_string);

  logger_.trace("Sending message on topic: {}", ndata_topic_);

  co_await mqtt_client_->send_message(ndata_topic_, payload_string, async_mqtt::qos::at_most_once);
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::make_will_payload() -> std::string {
  /// Construct MQTT will payload
  Payload payload;
  payload.set_timestamp(timestamp_milliseconds().count());

  auto* metric = payload.add_metrics();

  metric->set_name("bdSeq");
  metric->set_timestamp(timestamp_milliseconds().count());
  metric->set_datatype(8);

  metric->set_long_value(static_cast<uint64_t>(get_bd_seq()));

  std::string payload_string;
  payload.SerializeToString(&payload_string);

  return payload_string;
}

/// Create a timestamp in milliseconds from 1 Jan 1970 according to spec
template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::timestamp_milliseconds() -> std::chrono::milliseconds {
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;
  using std::chrono::system_clock;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::set_current_values(
    std::vector<tfc::ipc::details::any_slot_cb> const& metrics) -> void {
  nbirth_ = metrics;
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::send_current_values() -> void {
  if (mqtt_client_) {
    Payload payload;
    payload.set_timestamp(timestamp_milliseconds().count());
    seq_ = 0;
    payload.set_seq(seq_);

    auto* node_rebirth = payload.add_metrics();
    node_rebirth->set_name(constants::rebirth_metric.data());
    node_rebirth->set_timestamp(timestamp_milliseconds().count());
    node_rebirth->set_datatype(11);
    node_rebirth->set_boolean_value(false);
    node_rebirth->set_is_transient(false);
    node_rebirth->set_is_historical(false);
    node_rebirth->set_is_null(false);

    auto* bd_seq_metric = payload.add_metrics();

    bd_seq_metric->set_name("bdSeq");
    bd_seq_metric->set_timestamp(timestamp_milliseconds().count());
    bd_seq_metric->set_datatype(8);
    bd_seq_metric->set_long_value(0);

    for (auto const& signal_data : nbirth_) {
      auto* variable_metric = payload.add_metrics();

      std::string name;
      ipc::details::type_e type;

      std::visit(
          [&name, &type](auto&& receiver) {
            using receiver_t = std::remove_cvref_t<decltype(receiver)>;
            if constexpr (!std::same_as<receiver_t, std::monostate>) {
              name = format_signal_name(receiver->name().data());
              type = receiver->type().value_e;
            }
          },
          signal_data);

      variable_metric->set_name(format_signal_name(name));
      variable_metric->set_timestamp(timestamp_milliseconds().count());
      variable_metric->set_datatype(type_enum_convert(type));

      std::visit(
          [this, &variable_metric]<typename recv_t>(recv_t&& receiver) {
            using receiver_t = std::remove_cvref_t<decltype(receiver)>;
            if constexpr (std::same_as<receiver_t, ipc::details::bool_slot_cb_ptr>) {
              set_value_payload(variable_metric, receiver->value().value_or(false), logger_);
            } else if constexpr (std::same_as<receiver_t, ipc::details::int_slot_cb_ptr> ||
                                 std::same_as<receiver_t, ipc::details::uint_slot_cb_ptr> ||
                                 std::same_as<receiver_t, ipc::details::double_slot_cb_ptr>) {
              set_value_payload(variable_metric, receiver->value().value_or(0), logger_);
            } else if constexpr (std::same_as<receiver_t, ipc::details::string_slot_cb_ptr> ||
                                 std::same_as<receiver_t, ipc::details::json_slot_cb_ptr>) {
              set_value_payload(variable_metric, receiver->value().value_or(""), logger_);
            }
          },
          signal_data);

      variable_metric->set_is_transient(false);
      variable_metric->set_is_historical(false);
    }

    logger_.trace("NBIRTH payload: \n {}", payload.DebugString());

    std::string payload_string;
    payload.SerializeToString(&payload_string);

    const std::string topic = topic_formatter(
        { constants::namespace_element, config_.value().group_id, constants::nbirth, config_.value().node_id });

    mqtt_client_->set_initial_message(topic, payload_string, async_mqtt::qos::at_most_once);

    asio::co_spawn(mqtt_client_->strand(), mqtt_client_->send_message(topic, payload_string, async_mqtt::qos::at_most_once),
                   asio::detached);
  }
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::process_payload(async_mqtt::buffer const& data,
                                                                    async_mqtt::v5::publish_packet const& publish_packet)
    -> void {
  Payload payload;

  const bool payload_valid = payload.ParseFromArray(data.data(), static_cast<int>(data.size()));

  if (!payload_valid) {
    logger_.warn("Received invalid payload. Continuing to next iteration...");
    return;
  }

  logger_.trace("Payload parsed successfully. Processing payload...");

  if (payload.metrics_size() == 0) {
    logger_.trace("No metrics inside of payload. Nothing more to do.");
  }

  auto metric = payload.metrics(0);

  logger_.trace("Incoming NCMD payload: \n {}", payload.DebugString());

  if (payload.has_seq()) {
    logger_.error("NCMD payload should not have a seq nr timestamp but it does.");
  }

  if (!payload.has_timestamp()) {
    logger_.error("NCMD payload should have timestamp but it doesn't.");
  }

  if (publish_packet.opts().get_qos() != async_mqtt::qos::at_most_once) {
    logger_.error("NCMD payload should have QOS set to 0 but it doesn't.");
  }

  if (publish_packet.opts().get_retain() != async_mqtt::pub::retain::no) {
    logger_.error("NCMD payload should have retain set to false but it doesn't");
  }

  if (metric.name() == constants::rebirth_metric) {
    logger_.trace("NBIRTH requested.");
    send_current_values();
  } else {
    logger_.trace("Metric received, updating value.");
    set_metric_callback(metric);
  }
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::set_value_change_callback(
    std::function<void(std::string, std::variant<bool, double, std::string, int64_t, uint64_t>)> const& callback) -> void {
  value_change_callback_ = callback;
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::set_metric_callback(Payload_Metric const& metric) -> void {
  if (value_change_callback_.has_value()) {
    if (metric.has_boolean_value()) {
      (value_change_callback_.value())(metric.name(), metric.boolean_value());
    } else if (metric.has_double_value()) {
      (value_change_callback_.value())(metric.name(), metric.double_value());
    } else if (metric.has_float_value()) {
      (value_change_callback_.value())(metric.name(), metric.float_value());
    } else if (metric.has_int_value()) {
      (value_change_callback_.value())(metric.name(), static_cast<uint64_t>(metric.int_value()));
    } else if (metric.has_long_value()) {
      (value_change_callback_.value())(metric.name(), metric.long_value());
    } else if (metric.has_string_value()) {
      (value_change_callback_.value())(metric.name(), metric.string_value());
    }
  }
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::strand() -> asio::strand<asio::any_io_executor> {
  return mqtt_client_->strand();
}

/// This function converts tfc types to Spark Plug B types
/// More information can be found (page 76) in the spec under section 6.4.16 data types:
/// https://sparkplug.eclipse.org/specification/version/3.0/documents/sparkplug-specification-3.0.0.pdf
template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::type_enum_convert(tfc::ipc::details::type_e type) -> DataType {
  using enum tfc::ipc::details::type_e;
  switch (type) {
    case unknown:
      return DataType::Unknown;
    case _bool:
      return DataType::Boolean;
    case _int64_t:
      return DataType::Int64;
    case _uint64_t:
      return DataType::UInt64;
    case _double_t:
      return DataType::Double;
    case _string:
    case _json:
      return DataType::String;
  }
  return DataType::Unknown;
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::set_value_payload(Payload_Metric* metric,
                                                                      std::optional<std::any> const& value,
                                                                      tfc::logger::logger const& logger) -> void {
  if (value.has_value()) {
    if (value.value().type() == typeid(bool)) {
      metric->set_boolean_value(std::any_cast<bool>(value.value()));
    } else if (value.value().type() == typeid(std::string)) {
      metric->set_string_value(std::any_cast<std::string>(value.value()));
    } else if (value.value().type() == typeid(uint64_t)) {
      metric->set_long_value(std::any_cast<uint64_t>(value.value()));
    } else if (value.value().type() == typeid(int64_t)) {
      metric->set_long_value(std::any_cast<int64_t>(value.value()));
    } else if (value.value().type() == typeid(double)) {
      metric->set_double_value(std::any_cast<double>(value.value()));
    } else if (value.value().type() == typeid(float)) {
      metric->set_float_value(std::any_cast<float>(value.value()));
    } else if (value.value().type() == typeid(uint32_t)) {
      metric->set_int_value(std::any_cast<uint32_t>(value.value()));
    } else {
      logger.error("Unknown type: {}", value.value().type().name());
    }
  } else {
    metric->set_is_null(true);
  }
}

/// Spark Plug B disallows the use of some special characters in the signal name.
template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::format_signal_name(std::string signal_name_to_format) -> std::string {
  for (const auto& forbidden_char : { '+', '#', '-', '/' }) {
    std::ranges::replace(signal_name_to_format.begin(), signal_name_to_format.end(), forbidden_char, '_');
  }

  std::ranges::replace(signal_name_to_format.begin(), signal_name_to_format.end(), '.', '/');
  return signal_name_to_format;
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::topic_formatter(std::vector<std::string_view> const& topic_vector)
    -> std::string {
  std::string topic;
  for (auto const& topic_element : topic_vector) {
    topic += topic_element;
    topic += "/";
  }
  topic.pop_back();
  return topic;
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::connect_mqtt_client() -> asio::awaitable<bool> {
  co_return co_await mqtt_client_->connect();
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::subscribe_to_ncmd() -> asio::awaitable<bool> {
  co_return co_await mqtt_client_->subscribe_to_topic(ncmd_topic_);
}

template <class config_t, class mqtt_client_t>
auto spark_plug_interface<config_t, mqtt_client_t>::wait_for_payloads(
    std::function<void(async_mqtt::buffer const& data, async_mqtt::v5::publish_packet publish_packet)> process_payload,
    bool& restart_needed) -> asio::awaitable<void> {
  co_await mqtt_client_->wait_for_payloads(process_payload, restart_needed);
}

}  // namespace tfc::mqtt

template class tfc::mqtt::spark_plug_interface<
    tfc::mqtt::config::spark_plug_b_mock,
    tfc::mqtt::client<tfc::mqtt::endpoint_client_mock, tfc::mqtt::config::broker_mock>>;

template class tfc::mqtt::spark_plug_interface<
    tfc::confman::config<tfc::mqtt::config::spark_plug_b>,
    tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::confman::config<tfc::mqtt::config::broker>>>;
