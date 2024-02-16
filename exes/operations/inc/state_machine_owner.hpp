#pragma once
#include <memory>
#include <system_error>

#include <boost/sml.hpp>

#include <tfc/confman.hpp>
#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/dbus/sml_interface.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/operation_mode/common.hpp>
#include <tfc/sml_logger.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/utils/asio_fwd.hpp>
#include <tfc/utils/pragmas.hpp>

namespace tfc::operation {

namespace asio = boost::asio;
using new_mode = mode_e;
using old_mode = mode_e;

namespace detail {
template <typename owner_t>
struct state_machine;

struct storage {
  std::optional<std::chrono::milliseconds> startup_time{};
  std::optional<std::chrono::milliseconds> stopping_time{};
  struct glaze {
    // clang-format off
    static constexpr auto value{ glz::object(
        "startup_time", &storage::startup_time, "Delay to run initial sequences to get the equipment ready.",
        "stopping_time", &storage::stopping_time, "Delay to run ending sequences to get the equipment ready for being stopped.") };
    // clang-format on
    static constexpr std::string_view name{ "state_machine" };
  };
};

}  // namespace detail

template <template <typename description_t, typename manager_client_t = ipc_ruler::ipc_manager_client&> typename signal_t =
              ipc::signal,
          template <typename description_t, typename manager_client_t = ipc_ruler::ipc_manager_client&> typename slot_t =
              ipc::slot,
          template <typename, typename...> typename sml_t = boost::sml::sm>
class state_machine_owner {
public:
  explicit state_machine_owner(asio::io_context&, std::shared_ptr<sdbusplus::asio::connection>);

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

  void on_new_mode(tfc::stx::invocable<new_mode, old_mode> auto&& callback) {
    on_new_state_ = std::forward<decltype(callback)>(callback);
  }
  void transition(mode_e new_mode, mode_e old_mode);

  // accessors for testing
  auto sm() const noexcept { return states_; }
  auto stopped_signal() const noexcept -> auto const& { return stopped_; }
  auto starting_signal() const noexcept -> auto const& { return starting_; }
  auto running_signal() const noexcept -> auto const& { return running_; }
  auto stopping_signal() const noexcept -> auto const& { return stopping_; }
  auto cleaning_signal() const noexcept -> auto const& { return cleaning_; }
  auto emergency_signal() const noexcept -> auto const& { return emergency_out_; }
  auto mode_signal() const noexcept -> auto const& { return mode_; }
  auto mode_str_signal() const noexcept -> auto const& { return mode_str_; }
  auto stop_reason_str_signal() const noexcept -> auto const& { return stop_reason_str_; }
  auto set_stop_reason(const std::string& reason) -> void {
    stop_reason_str_.async_send(reason, [this](const std::error_code& err, std::size_t) {
      if (err) {
        logger_.error("Error sending stop reason: '{}'", err.message());
      }
    });
  }

  void on_starting_timer_expired(std::error_code const&);
  void on_stopping_timer_expired(std::error_code const&);

  void starting_finished_new_state(bool);
  void stopping_finished_new_state(bool);
  void running_new_state(bool);
  void cleaning_new_state(bool);
  void maintenance_new_state(bool);

  void emergency(bool);
  void fault(bool);

private:
  std::function<void(new_mode, old_mode)> on_new_state_{};
  asio::io_context& ctx_;
  std::shared_ptr<sdbusplus::asio::connection> dbus_;

  using bool_signal_t = signal_t<ipc::details::type_bool>;
  using bool_slot_t = slot_t<ipc::details::type_bool>;
  using string_signal_t = signal_t<ipc::details::type_string>;
  using uint_signal_t = signal_t<ipc::details::type_uint>;
  ipc_ruler::ipc_manager_client mclient_{ dbus_ };
  bool_signal_t stopped_{ ctx_, mclient_, "stopped" };
  bool_signal_t starting_{ ctx_, mclient_, "starting" };
  bool_signal_t running_{ ctx_, mclient_, "running" };
  bool_signal_t stopping_{ ctx_, mclient_, "stopping" };
  bool_signal_t cleaning_{ ctx_, mclient_, "cleaning" };
  bool_signal_t emergency_out_{ ctx_, mclient_, "emergency_out" };
  bool_signal_t fault_out_{ ctx_, mclient_, "fault" };
  uint_signal_t mode_{ ctx_, mclient_, "mode" };
  string_signal_t mode_str_{ ctx_, mclient_, "mode" };
  string_signal_t stop_reason_str_{ ctx_, mclient_, "stop_reason" };
  bool_slot_t starting_finished_{ ctx_, mclient_, "starting_finished",
                                  std::bind_front(&state_machine_owner::starting_finished_new_state, this) };
  bool_slot_t stopping_finished_{ ctx_, mclient_, "stopping_finished",
                                  std::bind_front(&state_machine_owner::stopping_finished_new_state, this) };
  bool_slot_t run_button_{ ctx_, mclient_, "run_button", std::bind_front(&state_machine_owner::running_new_state, this) };
  bool_slot_t cleaning_button_{ ctx_, mclient_, "cleaning_button",
                                std::bind_front(&state_machine_owner::cleaning_new_state, this) };
  bool_slot_t maintenance_button_{ ctx_, mclient_, "maintenance_button",
                                   std::bind_front(&state_machine_owner::maintenance_new_state, this) };
  bool_slot_t emergency_{ ctx_, mclient_, "emergency", "Emergency active input",
                          std::bind_front(&state_machine_owner::emergency, this) };
  bool_slot_t fault_{ ctx_, mclient_, "fault", "Fault active, like trip signal from motor driver",
                      std::bind_front(&state_machine_owner::fault, this) };
  tfc::logger::logger logger_{ "state_machine" };
  tfc::confman::config<detail::storage> config_{ ctx_, "state_machine",
                                                 detail::storage{ .startup_time = std::chrono::milliseconds{ 0 },
                                                                  .stopping_time = std::chrono::milliseconds{ 0 } } };
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
  tfc::dbus::sml::interface sml_interface_ {
    dbus_interface_
  };
  using state_machine_t = sml_t<detail::state_machine<state_machine_owner>, boost::sml::logger<tfc::dbus::sml::interface>>;
  std::shared_ptr<state_machine_t> states_;
};

extern template class state_machine_owner<>;

}  // namespace tfc::operation
