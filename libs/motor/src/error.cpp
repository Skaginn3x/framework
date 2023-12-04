#include <string>
#include <tfc/motors/errors.hpp>
#include <tfc/utils/pragmas.hpp>

namespace tfc::motor {
namespace errors {
namespace {
class motor_error_category_t : public std::error_category{ [[nodiscard]] auto name() const noexcept -> char const *
                                                           override{ return "tfc::motors::error";
}
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
  }
  return "unknown error: " + std::to_string(error);
}
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
}  // namespace errors
category_instance;
}  // namespace tfc::motor
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
// clang-format on
}
/// The error category for modbus errors.
auto category() -> std::error_category const& {
  return errors::category_instance;
}
}
