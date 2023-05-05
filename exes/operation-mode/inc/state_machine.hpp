#include <system_error>

#include <tfc/logger.hpp>
#include <tfc/operation_mode/common.hpp>

namespace tfc::operation {

class state_machine {
public:
  state_machine() : logger_("state_machine") {}

  std::error_code try_set_mode(tfc::operation::mode_e new_mode) {
    logger_.trace("Got new mode: {}", tfc::operation::mode_e_str(new_mode));
    // todo
    current_mode_ = new_mode;
    return {};
  }

  [[nodiscard]] auto get_mode() const noexcept -> tfc::operation::mode_e { return current_mode_; }

private:
  tfc::operation::mode_e current_mode_{ tfc::operation::mode_e::unknown };
  tfc::logger::logger logger_;
};

}  // namespace tfc::operation
