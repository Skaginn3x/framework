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
enum struct err_enum : int {
  no_motor_configured,
  motor_not_connected,
  motor_general_error,
  motor_tripped,
  motor_missing_speed_reference,
};
}

/// The error category for Modbus errors.
std::error_category const& category();

/// Get an error code for a Modbus error,
inline std::error_code motor_error(errors::err_enum error) {
  int error_int = static_cast<int>(error);
  return std::error_code(error_int, category());
}
}  // namespace tfc::motor
