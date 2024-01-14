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
    static constexpr std::string_view name{ "state_machine" };
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

  template <typename callback_t>
  void on_new_mode(callback_t&& callback)
    requires std::invocable<std::remove_cvref_t<callback_t>, new_mode, old_mode>
  {
    on_new_state_ = std::forward<callback_t>(callback);
  }
  void transition(mode_e new_mode, mode_e old_mode) const;

private:
  void starting_finished_new_state(bool);
  void stopping_finished_new_state(bool);
  void running_new_state(bool);
  void cleaning_new_state(bool);
  void maintenance_new_state(bool);

  std::function<void(new_mode, old_mode)> on_new_state_{};
  boost::asio::io_context& ctx_;
  tfc::ipc_ruler::ipc_manager_client mclient_{ ctx_ };
  tfc::ipc::bool_signal stopped_{ ctx_, mclient_, "stopped" };
  tfc::ipc::bool_signal starting_{ ctx_, mclient_, "starting" };
  tfc::ipc::bool_signal running_{ ctx_, mclient_, "running" };
  tfc::ipc::bool_signal stopping_{ ctx_, mclient_, "stopping" };
  tfc::ipc::bool_signal cleaning_{ ctx_, mclient_, "cleaning" };
  tfc::ipc::uint_signal mode_{ ctx_, mclient_, "mode" };
  tfc::ipc::string_signal mode_str_{ ctx_, mclient_, "mode" };
  tfc::ipc::bool_slot starting_finished_{ ctx_, mclient_, "starting_finished",
                                          std::bind_front(&state_machine::starting_finished_new_state, this) };
  tfc::ipc::bool_slot stopping_finished_{ ctx_, mclient_, "stopping_finished",
                                          std::bind_front(&state_machine::stopping_finished_new_state, this) };
  tfc::ipc::bool_slot run_button_{ ctx_, mclient_, "run_button", std::bind_front(&state_machine::running_new_state, this) };
  tfc::ipc::bool_slot cleaning_button_{ ctx_, mclient_, "cleaning_button",
                                        std::bind_front(&state_machine::cleaning_new_state, this) };
  tfc::ipc::bool_slot maintenance_button_{ ctx_, mclient_, "maintenance_button",
                                           std::bind_front(&state_machine::maintenance_new_state, this) };
  tfc::logger::logger logger_{ "state_machine" };
  tfc::confman::config<detail::storage> config_{ ctx_, "state_machine" };
  std::shared_ptr<boost::sml::sm<detail::state_machine, boost::sml::logger<tfc::logger::sml_logger>>> states_;
};

}  // namespace tfc::operation
