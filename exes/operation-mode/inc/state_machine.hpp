#pragma once
#include <system_error>
#include <memory>

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


using boost::sml::literals::operator""_e;
using boost::sml::literals::operator""_s;

struct state_machine;

struct my_logger {
  template <class SM, class TEvent>
  void log_process_event(const TEvent&) {
    printf("[%s][process_event] %s\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TEvent>());
  }

  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard&, const TEvent&, bool result) {
    printf("[%s][guard] %s %s %s\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TGuard>(),
           boost::sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]"));
  }

  template <class SM, class TAction, class TEvent>
  void log_action(const TAction&, const TEvent&) {
    printf("[%s][action] %s %s\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TAction>(),
           boost::sml::aux::get_type_name<TEvent>());
  }

  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {
    printf("[%s][transition] %s -> %s\n", boost::sml::aux::get_type_name<SM>(), src.c_str(), dst.c_str());
  }
};

struct storage {
  std::optional<std::chrono::milliseconds> startup_time{};
  struct glaze {
    static constexpr auto value{ glz::object("startup_time", &storage::startup_time) };
    static constexpr auto name{ "state_machine_storage" };
  };
};

}  // namespace detail

class state_machine : public std::enable_shared_from_this<state_machine> {
public:
  static std::shared_ptr<state_machine> make(boost::asio::io_context& ctx) {
    return std::make_shared<state_machine>(ctx);
  }
//private: // todo
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

  [[nodiscard]] auto get_mode() const noexcept -> tfc::operation::mode_e { return current_mode_; }

  auto on_new_mode(std::invocable<new_mode, old_mode> auto&& callback) { on_new_state_ = callback; }

private:
  void running_new_state(bool);
  void cleaning_new_state(bool);
  void maintenance_new_state(bool);

  mode_e current_mode_{ tfc::operation::mode_e::unknown };
  std::function<void(new_mode, old_mode)> on_new_state_{};
  tfc::ipc::bool_signal stopped_;
  tfc::ipc::bool_signal starting_;
  tfc::ipc::bool_signal running_;
  tfc::ipc::bool_signal stopping_;
  tfc::ipc::bool_signal cleaning_;
  tfc::ipc::uint_signal mode_;
  tfc::ipc::string_signal mode_str_;
  tfc::ipc::bool_slot run_button_;
  tfc::ipc::bool_slot cleaning_button_;
  tfc::ipc::bool_slot maintenance_button_;
  tfc::logger::logger logger_;
  tfc::confman::config<detail::storage> config_;
  std::shared_ptr<boost::sml::sm<detail::state_machine, boost::sml::logger<detail::my_logger>>> states_;
  boost::asio::io_context& ctx_;
};

}  // namespace tfc::operation
