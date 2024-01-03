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
      case motor_tripped:
        return "Motor tripped";
      case motor_missing_home_reference:
        return "Motor msing home reference";
      case motor_missing_speed_reference:
        return "Motor missing speed reference";
      case motor_not_implemented:
        return "Motor function not implemented";
      case speedratio_out_of_range:
        return "Speed ratio out of range (-100% to 100%)";
      case positioning_unstable:
        return "Positioning unstable";
      case positioning_missing_event:
        return "Positioning missing event";
      case positioning_request_out_of_range:
        return "Positioning request out of range";
      case permission_denied:
        return "Permission to execute operation not allowed";
      case unknown:
        return "Unknown error";
    }
    return "unknown error: " + std::to_string(error);
  }
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
constinit motor_error_category_t category_instance{};
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
}  // namespace tfc::motor::errors

namespace tfc::motor {  // todo place in error namespace ?
/// The error category for modbus errors.
auto category() -> std::error_category const& {
  return category_instance;
}
}  // namespace tfc::motor
