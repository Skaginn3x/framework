#pragma once
#include <memory>
#include <system_error>

#include <boost/sml.hpp>

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/operation_mode/common.hpp>
#include <tfc/sml_logger.hpp>
#include <tfc/utils/pragmas.hpp>

namespace boost::asio {
class io_context;
}

namespace tfc::operation {

using new_mode = mode_e;
using old_mode = mode_e;

namespace detail {

using boost::sml::literals::operator""_e;
using boost::sml::literals::operator""_s;

struct state_machine;

struct storage {
  std::optional<std::chrono::milliseconds> startup_time{};
  std::optional<std::chrono::milliseconds> stopping_time{};
  struct glaze {
    // clang-format off
    static constexpr auto value{ glz::object(
        "startup_time", &storage::startup_time, "[ms] Delay to run initial sequences to get the equipment ready.",
        "stopping_time", &storage::stopping_time, "[ms] Delay to run ending sequences to get the equipment ready for being stopped.") };
    // clang-format on
    static constexpr auto name{ "state_machine" };
  };
};

}  // namespace detail

class state_machine {
public:
  explicit state_machine(boost::asio::io_context&);

  auto set_mode(tfc::operation::mode_e new_mode) -> std::error_code;

  void enter_stopped();
  void leave_stopped();
  void enter_starting();
  void leave_starting();
  void enter_running();
  void leave_running();
  void enter_stopping();
  void leave_stopping();
  void enter_cleaning();
  void leave_cleaning();
  void enter_emergency();
  void leave_emergency();
  void enter_fault();
  void leave_fault();
  void enter_maintenance();
  void leave_maintenance();

  auto on_new_mode(std::invocable<new_mode, old_mode> auto&& callback) { on_new_state_ = callback; }
  void transition(mode_e new_mode, mode_e old_mode) const;

private:
  void starting_finished_new_state(bool);
  void stopping_finished_new_state(bool);
  void running_new_state(bool);
  void cleaning_new_state(bool);
  void maintenance_new_state(bool);

  std::function<void(new_mode, old_mode)> on_new_state_{};
  tfc::ipc_ruler::ipc_manager_client mclient_;
  tfc::ipc::bool_signal stopped_;
  tfc::ipc::bool_signal starting_;
  tfc::ipc::bool_signal running_;
  tfc::ipc::bool_signal stopping_;
  tfc::ipc::bool_signal cleaning_;
  tfc::ipc::uint_signal mode_;
  tfc::ipc::string_signal mode_str_;
  tfc::ipc::bool_slot starting_finished_;
  tfc::ipc::bool_slot stopping_finished_;
  tfc::ipc::bool_slot run_button_;
  tfc::ipc::bool_slot cleaning_button_;
  tfc::ipc::bool_slot maintenance_button_;
  tfc::logger::logger logger_;
  tfc::confman::config<detail::storage> config_;
  std::shared_ptr<boost::sml::sm<detail::state_machine, boost::sml::logger<tfc::logger::sml_logger>>> states_;
  boost::asio::io_context& ctx_;
};

}  // namespace tfc::operation
