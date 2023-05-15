#pragma once
#include <system_error>

#include <boost/sml.hpp>

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/operation_mode/common.hpp>
#include <tfc/utils/pragmas.hpp>

namespace boost::asio {
class io_context;
}

namespace tfc::operation {

using new_mode = mode_e;
using old_mode = mode_e;

namespace detail {
struct state_machine;
}  // namespace detail

class state_machine {
public:
  explicit state_machine(boost::asio::io_context&);

  auto set_mode(tfc::operation::mode_e new_mode) -> std::error_code;

  void enter_stopped();
  void leave_stopped();
  void enter_warmup();
  void leave_warmup();
  void enter_running();
  void leave_running();
  void enter_cleaning();
  void leave_cleaning();
  void enter_emergency();
  void leave_emergency();
  void enter_fault();
  void leave_fault();
  void enter_maintenance();
  void leave_maintenance();

  [[nodiscard]] auto get_mode() const noexcept -> tfc::operation::mode_e { return current_mode_; }

  auto on_new_mode(std::invocable<new_mode, old_mode> auto&& callback) { on_new_state_ = callback; }

private:
  void running_new_state(bool);
  void cleaning_new_state(bool);
  void maintenance_new_state(bool);

  mode_e current_mode_{ tfc::operation::mode_e::unknown };
  std::function<void(new_mode, old_mode)> on_new_state_{};
  tfc::ipc::bool_signal stopped_;
  tfc::ipc::bool_signal warmup_;
  tfc::ipc::bool_signal running_;
  tfc::ipc::bool_signal cleaning_;
  tfc::ipc::uint_signal mode_;
  tfc::ipc::string_signal mode_str_;
  tfc::ipc::bool_slot run_button_;
  tfc::ipc::bool_slot cleaning_button_;
  tfc::ipc::bool_slot maintenance_button_;
  tfc::logger::logger logger_;
  std::shared_ptr<boost::sml::sm<detail::state_machine>> states_;
};

}  // namespace tfc::operation
