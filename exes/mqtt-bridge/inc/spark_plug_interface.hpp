#pragma once

#include <any>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>

#include <tfc/logger.hpp>

#include <constants.hpp>
#include <structs.hpp>

namespace tfc::mqtt {
namespace asio = boost::asio;

enum struct type_e : std::uint8_t;

using org::eclipse::tahu::protobuf::DataType;
using org::eclipse::tahu::protobuf::Payload;
using org::eclipse::tahu::protobuf::Payload_Metric;

template <class config_t, class mqtt_client_t>
class spark_plug_interface {
public:
  explicit spark_plug_interface(asio::io_context& io_ctx, config_t& config)
      : io_ctx_(io_ctx), config_(config),
        ncmd_topic_(topic_formatter(
            { constants::namespace_element, config_.value().group_id, constants::ncmd, config_.value().node_id })),
        mqtt_will_topic_(topic_formatter(
            { constants::namespace_element, config_.value().group_id, constants::ndeath, config_.value().node_id })),
        ndata_topic_(topic_formatter(
            { constants::namespace_element, config_.value().group_id, constants::ndata, config_.value().node_id })) {
    const std::string_view mqtt_will_payload{ make_will_payload() };

    mqtt_client_ = std::make_unique<mqtt_client_t>(io_ctx_, mqtt_will_topic_, mqtt_will_payload, config_);
  }

  auto make_will_payload() -> std::string {
    /// Construct MQTT will payload
    Payload payload;
    payload.set_timestamp(timestamp_milliseconds().count());

    auto* metric = payload.add_metrics();

    metric->set_name("bdSeq");
    metric->set_timestamp(timestamp_milliseconds().count());
    metric->set_datatype(DataType::UInt64);

    metric->set_long_value(static_cast<uint64_t>(get_bd_seq()));

    std::string payload_string;
    payload.SerializeToString(&payload_string);

    return payload_string;
  }

  auto get_bd_seq() -> int64_t { return bdSeq_++ % 256; }

  static auto timestamp_milliseconds() -> std::chrono::milliseconds {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::system_clock;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  }

  auto set_current_values(std::vector<structs::spark_plug_b_variable> const& metrics) -> void { variables_ = metrics; }

  auto send_current_values() -> void {
    if (mqtt_client_) {
      Payload payload;
      payload.set_timestamp(timestamp_milliseconds().count());
      seq_ = 0;
      payload.set_seq(seq_);

      auto* node_rebirth = payload.add_metrics();
      node_rebirth->set_name(constants::rebirth_metric.data());
      node_rebirth->set_timestamp(timestamp_milliseconds().count());
      node_rebirth->set_datatype(DataType::Boolean);
      node_rebirth->set_boolean_value(false);
      node_rebirth->set_is_transient(false);
      node_rebirth->set_is_historical(false);
      node_rebirth->set_is_null(false);

      auto* bd_seq_metric = payload.add_metrics();

      bd_seq_metric->set_name("bdSeq");
      bd_seq_metric->set_timestamp(timestamp_milliseconds().count());
      bd_seq_metric->set_datatype(DataType::UInt64);
      bd_seq_metric->set_long_value(0);

      for (auto const& variable : variables_) {
        auto* variable_metric = payload.add_metrics();

        auto* metadata = variable_metric->mutable_metadata();
        metadata->set_description(variable.description);

        variable_metric->set_name(variable.name);
        variable_metric->set_datatype(variable.datatype);
        variable_metric->set_timestamp(timestamp_milliseconds().count());
        set_value_payload(variable_metric, variable.value, logger_);

        variable_metric->set_is_transient(false);
        variable_metric->set_is_historical(false);
      }

      logger_.trace("NBIRTH payload: \n {}", payload.DebugString());

      std::string payload_string;
      payload.SerializeToString(&payload_string);

      const std::string topic = topic_formatter(
          { constants::namespace_element, config_.value().group_id, constants::nbirth, config_.value().node_id });

      mqtt_client_->set_initial_message(topic, payload_string, async_mqtt::qos::at_most_once);

      asio::co_spawn(mqtt_client_->strand(),
                     mqtt_client_->send_message(topic, payload_string, async_mqtt::qos::at_most_once), asio::detached);
    }
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

  auto update_value(structs::spark_plug_b_variable& variable) -> void {
    asio::co_spawn(strand(), update_value_impl(variable), asio::detached);
  }

  auto update_value_impl(structs::spark_plug_b_variable& variable) -> asio::awaitable<void> {
    Payload payload = make_payload();
    Payload_Metric* metric = payload.add_metrics();

    if (metric == nullptr) {
      logger_.error("Failed to add metric to payload for variable: {}", variable.name);
      co_return;
    }

    metric->set_name(variable.name);
    metric->set_timestamp(timestamp_milliseconds().count());
    metric->set_datatype(variable.datatype);

    if (variable.value.has_value()) {
      set_value_payload(metric, variable.value.value(), logger_);
    } else {
      metric->set_is_null(true);
    }

    auto* metadata = metric->mutable_metadata();
    metadata->set_description(variable.description);

    logger_.trace("Updating variable: {}", variable.name);
    logger_.trace("Payload: {}", payload.DebugString());

    std::string payload_string;
    payload.SerializeToString(&payload_string);

    logger_.trace("Sending message on topic: {}", ndata_topic_);

    co_await mqtt_client_->send_message(ndata_topic_, payload_string, async_mqtt::qos::at_most_once);
  }

  auto set_value_change_callback(
      std::function<void(std::string, std::variant<bool, double, std::string, int64_t, uint64_t>)> const& callback) -> void {
    value_change_callback_ = callback;
  }

  auto process_payload(async_mqtt::buffer const& data, async_mqtt::v5::publish_packet const& publish_packet) -> void {
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

  auto set_metric_callback(Payload_Metric const& metric) -> void {
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

  auto strand() -> asio::strand<asio::any_io_executor> { return mqtt_client_->strand(); }

  static auto set_value_payload(Payload_Metric* metric, std::optional<std::any> const& value, logger::logger const& logger)
      -> void {
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

  static auto topic_formatter(std::vector<std::string_view> const& topic_vector) -> std::string {
    std::string topic;
    for (auto const& topic_element : topic_vector) {
      topic += topic_element;
      topic += "/";
    }
    topic.pop_back();
    return topic;
  }

  auto connect_mqtt_client() -> asio::awaitable<bool> { co_return co_await mqtt_client_->connect(); }

  auto subscribe_to_ncmd() -> asio::awaitable<bool> { co_return co_await mqtt_client_->subscribe_to_topic(ncmd_topic_); }

  auto wait_for_payloads(
      std::function<void(async_mqtt::buffer const& data, async_mqtt::v5::publish_packet publish_packet)> process_payload
      ) -> asio::awaitable<void> {
    co_await mqtt_client_->wait_for_payloads(process_payload);
  }

private:
  asio::io_context& io_ctx_;
  config_t& config_;
  std::unique_ptr<mqtt_client_t> mqtt_client_;
  std::vector<structs::spark_plug_b_variable> variables_;
  uint64_t seq_ = 1;
  logger::logger logger_{ "spark_plug_interface" };
  std::optional<std::function<void(std::string, std::variant<bool, double, std::string, int64_t, uint64_t>)> >
      value_change_callback_;
  std::string ncmd_topic_;
  std::string mqtt_will_topic_;
  std::string ndata_topic_;
  int64_t bdSeq_ = 0;
};
}  // namespace tfc::mqtt
