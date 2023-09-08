#pragma once

#include <array>
#include <string>

using org::eclipse::tahu::protobuf::Payload;

namespace tfc::mqtt::impl {

/// This function converts tfc types to Spark Plug B types
/// More information can be found (page 76) in the spec under section 6.4.16 data types:
/// https://sparkplug.eclipse.org/specification/version/3.0/documents/sparkplug-specification-3.0.0.pdf
auto type_enum_convert(tfc::ipc::details::type_e type) -> uint32_t {
  switch (type) {
    case tfc::ipc::details::type_e::unknown:
      return 0;
    case tfc::ipc::details::type_e::_bool:
      return 11;
    case tfc::ipc::details::type_e::_int64_t:
      return 4;
    case tfc::ipc::details::type_e::_uint64_t:
      return 8;
    case tfc::ipc::details::type_e::_double_t:
      return 10;
    case tfc::ipc::details::type_e::_string:
    case tfc::ipc::details::type_e::_json:
      return 12;
  }
  return 0;
}

/// Spark Plug B disallows the use of some special characters in the signal name.
// TODO: Add all of the chars and make this complete.
static const std::array forbidden_chars = { '.' };
auto format_signal_name(std::string signal_name_to_format) -> std::string {
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

}  // namespace tfc::mqtt::impl
