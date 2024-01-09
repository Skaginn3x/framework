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
  success = 0,                // todo refactor to none ?
  no_motor_configured = 1,
  motor_not_connected = 2,
  motor_general_error = 3,
  motor_tripped = 4,
  motor_missing_speed_reference = 5,
  motor_missing_home_reference = 6,
  motor_not_implemented = 7,
  motor_method_not_implemented = 8,
  speedratio_out_of_range = 11,
  positioning_unstable = 20,
  positioning_missing_event = 21,
  positioning_request_out_of_range = 22,
  permission_denied = 30,
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
inline std::error_code motor_error(errors::err_enum error) {
  auto const error_int = static_cast<int>(error);
  return std::error_code(error_int, category());
}

/// Get the motor error enum from a std::error_code.
inline errors::err_enum motor_enum(std::error_code err) {
  using enum errors::err_enum;
  if (err.category() == category()) {
    return errors::enum_cast(err.value()).value_or(unknown);
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
