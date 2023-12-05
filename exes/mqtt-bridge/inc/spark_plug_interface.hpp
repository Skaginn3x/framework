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

#include <tfc/confman.hpp>
// #include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/utils/asio_fwd.hpp>

#include <client.hpp>
#include <config/broker.hpp>
#include <config/spark_plug_b.hpp>
#include <config/spark_plug_b_mock.hpp>
#include <structs.hpp>

namespace async_mqtt {
class buffer;
namespace v5 {
template <std::size_t packet_id_bytes>
class basic_publish_packet;

using publish_packet = basic_publish_packet<2>;
}  // namespace v5
}  // namespace async_mqtt

namespace org::eclipse::tahu::protobuf {
enum DataType : int;  // NOLINT

class Payload;

class Payload_Metric;
}  // namespace org::eclipse::tahu::protobuf

namespace tfc::mqtt {

namespace asio = boost::asio;

enum struct type_e : std::uint8_t;

using org::eclipse::tahu::protobuf::DataType;
using org::eclipse::tahu::protobuf::Payload;
using org::eclipse::tahu::protobuf::Payload_Metric;

// struct spark_plug_b_variable;

template <class config_t, class mqtt_client_t>
class spark_plug_interface {
public:
  explicit spark_plug_interface(asio::io_context& io_ctx);

  auto make_will_payload() -> std::string;

  auto get_bd_seq() -> int64_t;

  static auto timestamp_milliseconds() -> std::chrono::milliseconds;

  auto set_current_values(std::vector<spark_plug_b_variable> const& variables) -> void;

  auto send_current_values() -> void;

  auto make_payload() -> Payload;

  auto update_value(spark_plug_b_variable& variable) -> void;

  auto update_value_impl(spark_plug_b_variable& variable) -> asio::awaitable<void>;

  auto set_value_change_callback(
      std::function<void(std::string, std::variant<bool, double, std::string, int64_t, uint64_t>)> const& callback) -> void;

  auto process_payload(async_mqtt::buffer const& data, async_mqtt::v5::publish_packet const& publish_packet) -> void;

  auto set_metric_callback(Payload_Metric const& metric) -> void;

  auto strand() -> asio::strand<asio::any_io_executor>;


  static auto set_value_payload(Payload_Metric*, std::optional<std::any> const& value, tfc::logger::logger const&) -> void;

  static auto format_signal_name(std::string signal_name_to_format) -> std::string;

  static auto topic_formatter(std::vector<std::string_view> const& topic_vector) -> std::string;

  auto connect_mqtt_client() -> asio::awaitable<bool>;

  auto subscribe_to_ncmd() -> asio::awaitable<bool>;

  auto wait_for_payloads(
      std::function<void(async_mqtt::buffer const& data, async_mqtt::v5::publish_packet publish_packet)> process_payload,
      bool& restart_needed) -> asio::awaitable<void>;

private:
  asio::io_context& io_ctx_;
  config_t config_{ io_ctx_, "spark_plug_b_config" };
  std::unique_ptr<mqtt_client_t> mqtt_client_;
  std::vector<spark_plug_b_variable> variables_;
  uint64_t seq_ = 1;
  tfc::logger::logger logger_{ "spark_plug_interface" };
  std::optional<std::function<void(std::string, std::variant<bool, double, std::string, int64_t, uint64_t>)>>
      value_change_callback_;
  std::string ncmd_topic_;
  std::string mqtt_will_topic_;
  std::string ndata_topic_;
  int64_t bdSeq_ = 0;
};

using spark_plug = spark_plug_interface<tfc::confman::config<config::spark_plug_b>, client_n>;
using spark_plug_mock = spark_plug_interface<config::spark_plug_b_mock, client_mock>;

extern template class tfc::mqtt::spark_plug_interface<
    tfc::mqtt::config::spark_plug_b_mock,
    tfc::mqtt::client<tfc::mqtt::endpoint_client_mock, tfc::mqtt::config::broker_mock>>;

extern template class tfc::mqtt::spark_plug_interface<
    tfc::confman::config<tfc::mqtt::config::spark_plug_b>,
    tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::confman::config<tfc::mqtt::config::broker>>>;

}  // namespace tfc::mqtt
