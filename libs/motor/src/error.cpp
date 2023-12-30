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
      case positioning_unstable:
        return "Positioning unstable";
      case positioning_missing_event:
        return "Positioning missing event";
      case positioning_request_out_of_range:
        return "Positioning request out of range";
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
}  // namespace tfc::motor::errors

namespace tfc::motor {
/// The error category for modbus errors.
auto category() -> std::error_category const& {
  return category_instance;
}
}  // namespace tfc::motor
