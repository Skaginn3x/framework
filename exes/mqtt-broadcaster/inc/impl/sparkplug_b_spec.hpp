#pragma once

#include <array>
#include <string>

using org::eclipse::tahu::protobuf::Payload;
using org::eclipse::tahu::protobuf::Payload_Metric;

namespace tfc::mqtt::impl {

/// This function converts tfc types to Spark Plug B types
/// More information can be found (page 76) in the spec under section 6.4.16 data types:
/// https://sparkplug.eclipse.org/specification/version/3.0/documents/sparkplug-specification-3.0.0.pdf
auto type_enum_convert(tfc::ipc::details::type_e type) -> uint32_t {
  using enum tfc::ipc::details::type_e;
  switch (type) {
    case unknown:
      return 0;
    case _bool:
      return 11;
    case _int64_t:
      return 4;
    case _uint64_t:
      return 8;
    case _double_t:
      return 10;
    case _string:
    case _json:
      return 12;
  }
  return 0;
}

/// Spark Plug B disallows the use of some special characters in the signal name.
auto format_signal_name(std::string signal_name_to_format) -> std::string {
  static const std::array forbidden_chars = { '+', '#', '-', '/' };
  for (const auto& forbidden_char : forbidden_chars) {
    std::ranges::replace(signal_name_to_format.begin(), signal_name_to_format.end(), forbidden_char, '_');
  }

  std::replace(signal_name_to_format.begin(), signal_name_to_format.end(), '.', '/');
  return signal_name_to_format;
}

// Create a timestamp in milliseconds from 1 Jan 1970 according to spec
static auto timestamp_milliseconds() -> std::chrono::milliseconds {
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;
  using std::chrono::system_clock;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}

auto make_payload(uint64_t seq_) -> Payload {
  Payload payload;
  payload.set_timestamp(impl::timestamp_milliseconds().count());
  if (seq_ == 255) {
    seq_ = 0;
  } else {
    seq_++;
  }

  payload.set_seq(seq_);

  return payload;
}

auto set_value_payload(Payload_Metric* metric, std::any value, tfc::logger::logger& logger) -> void {
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
    logger.error("Unknown type: {}", value.type().name());
  }
}

auto set_value_payload(Payload_Metric* metric, const bool value) -> void {
  metric->set_boolean_value(value);
}

auto set_value_payload(Payload_Metric* metric, const std::string value) -> void {
  metric->set_string_value(value);
}

auto set_value_payload(Payload_Metric* metric, const uint64_t value) -> void {
  metric->set_long_value(value);
}

auto set_value_payload(Payload_Metric* metric, const int64_t value) -> void {
  metric->set_long_value(value);
}

auto set_value_payload(Payload_Metric* metric, const double value) -> void {
  metric->set_double_value(value);
}

auto set_value_payload(Payload_Metric* metric, const float value) -> void {
  metric->set_float_value(value);
}

auto set_value_payload(Payload_Metric* metric, const uint32_t value) -> void {
  metric->set_int_value(value);
}

std::string port_to_string(const std::variant<mqtt::port_e, uint16_t>& port) {
  return std::visit([](auto&& arg) { return std::to_string(static_cast<uint16_t>(std::forward<decltype(arg)>(arg))); },
                    port);
}

}  // namespace tfc::mqtt::impl
