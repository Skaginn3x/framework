#pragma once
#include <system_error>
namespace tfc::motor {

// Predefined motor errors

// TODO: think about this some more.
// how can we allow individual vfds or
// servo controllers to express their own
// error codes.

// I don't really believe
// that control level code cares
// 'what' is wrong with each motor.,
// just that it does not turn.
// So this could maybe be exposed in
// a dbus api for the hmi and scada or
// inside some alarm manager.
namespace errors {
enum struct err_enum : int {  // todo refactor to error_code_e ? or just error_e
  motor_method_not_implemented = -2,
  motor_not_implemented = -1,
  success = 0,
  no_motor_configured = 1,
  motor_not_connected = 2,
  motor_general_error = 3,
  frequency_drive_communication_fault = 4,
  motor_missing_speed_reference = 5,
  motor_missing_home_reference = 6,
  motor_home_sensor_unconfigured = 7,
  speedratio_out_of_range = 11,
  positioning_unstable = 20,
  positioning_missing_event = 21,
  positioning_AA_BB_events = 22,
  positioning_request_out_of_range = 23,
  positioning_positive_limit_reached = 24,
  positioning_negative_limit_reached = 25,
  permission_denied = 30,
  operation_canceled = 31,  // only used between client and server, user should use std::errc::operation_canceled
  frequency_drive_reports_fault = 32,
  unknown = 100,
};

auto enum_name(err_enum) noexcept -> std::string_view;
auto enum_cast(std::underlying_type_t<err_enum>) noexcept -> std::optional<err_enum>;
auto enum_cast(std::string_view) noexcept -> std::optional<err_enum>;

inline auto format_as(err_enum err) -> std::string_view {  // for fmt
  return enum_name(err);
}

}  // namespace errors

/// The error category for Motor errors.
std::error_category const& category();

/// Get an error code for a Motor error.
inline std::error_code motor_error(const errors::err_enum error) {
  auto const error_int = static_cast<int>(error);
  return std::error_code(error_int, category());
}

/// Get the motor error enum from a std::error_code.
inline errors::err_enum motor_enum(const std::error_code err) {
  using enum errors::err_enum;
  if (err.category() == category()) {
    return errors::enum_cast(err.value()).value_or(unknown);
  }
  if (err == std::errc::operation_canceled) {
    return operation_canceled;
  }
  if (!err) {
    return success;
  }
  return unknown;
}
}  // namespace tfc::motor

namespace sdbusplus::message::details {
template <typename value_t>
struct convert_from_string;

template <typename value_t>
struct convert_to_string;

template <>
struct convert_from_string<tfc::motor::errors::err_enum> {
  using enum_t = tfc::motor::errors::err_enum;
  static auto op(const std::string& err_str) noexcept -> std::optional<enum_t> {
    return tfc::motor::errors::enum_cast(err_str);
  }
};

template <>
struct convert_to_string<tfc::motor::errors::err_enum> {
  using enum_t = tfc::motor::errors::err_enum;
  static auto op(enum_t err) -> std::string { return std::string{ enum_name(err) }; }
};
}  // namespace sdbusplus::message::details
