#include <string>

#include <magic_enum.hpp>

#include <tfc/motor/errors.hpp>
#include <tfc/utils/pragmas.hpp>

namespace {
using namespace tfc::motor::errors;

class motor_error_category_t : public std::error_category {
  [[nodiscard]] auto name() const noexcept -> char const* override { return "tfc::motors::error"; }
  [[nodiscard]] auto message(int error) const noexcept -> std::string override {
    switch (static_cast<err_enum>(error)) {
      using enum err_enum;
      case success:
        return "Success";
      case no_motor_configured:
        return "No motor configured";
      case motor_not_connected:
        return "Motor not connected";
      case motor_general_error:
        return "Motor general error";
      case frequency_drive_communication_fault:
        return "Motor communication fault";
      case motor_missing_home_reference:
        return "Motor missing home reference";
      case motor_missing_speed_reference:
        return "Motor missing speed reference";
      case motor_not_implemented:
        return "Motor function not implemented";
      case motor_method_not_implemented:
        return "Motor method not implemented";
      case motor_home_sensor_unconfigured:
        return "Missing configured home speed or home sensor disconnected";
      case speedratio_out_of_range:
        return "Speed ratio out of range (-100% to 100%)";
      case positioning_unstable:
        return "Positioning unstable";
      case positioning_missing_event:
        return "Positioning missing event";
      case positioning_AA_BB_events:
        return "Positioning pair of AA events and BB events, should be ABABAB ...";
      case positioning_request_out_of_range:
        return "Positioning request out of range";
      case positioning_positive_limit_reached:
        return "Positioning positive limit reached";
      case positioning_negative_limit_reached:
        return "Positioning negative limit reached";
      case permission_denied:
        return "Permission to execute operation not allowed";
      case operation_canceled:
        return "Operation canceled";
      case frequency_drive_reports_fault:
        return "Frequency drive reports fault";
      case unknown:
        return "Unknown error";
    }
    return "unknown error: " + std::to_string(error);
  }
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
thread_local motor_error_category_t category_instance{};
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
// clang-format on
}  // namespace

namespace tfc::motor::errors {
auto enum_name(err_enum error) noexcept -> std::string_view {
  return magic_enum::enum_name(error);
}
auto enum_cast(std::underlying_type_t<err_enum> error) noexcept -> std::optional<err_enum> {
  return magic_enum::enum_cast<err_enum>(error);
}
auto enum_cast(std::string_view error) noexcept -> std::optional<err_enum> {
  return magic_enum::enum_cast<err_enum>(error);
}
}  // namespace tfc::motor::errors

namespace tfc::motor {  // todo place in error namespace ?
/// The error category for modbus errors.
auto category() -> std::error_category const& {
  return category_instance;
}
}  // namespace tfc::motor
