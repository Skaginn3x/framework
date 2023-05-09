#include "state_machine.hpp"
#include <boost/asio.hpp>

namespace tfc::operation {

namespace detail {

using boost::sml::literals::operator""_e;
using boost::sml::literals::operator""_s;

struct state_machine {
  explicit state_machine(tfc::operation::state_machine& owner) : owner_{ owner } {}

  auto operator()() {
    using boost::sml::_;
    using boost::sml::on_entry;
    using boost::sml::on_exit;

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
            * "stopped"_s + on_entry<_> / [this](){ owner_.enter_stopped(); }
            , "stopped"_s + on_exit<_> / [this](){ owner_.leave_stopped(); }
            , "stopped"_s + "run_button"_e = "warmup"_s
            , "stopped"_s + "set_mode_run"_e = "warmup"_s
            , "warmup"_s + on_entry<_> / [this](){ owner_.enter_warmup(); }
            , "warmup"_s + on_exit<_> / [this](){ owner_.leave_warmup(); }
            , "warmup"_s + "warmup_timeout"_e = "running"_s
            , "warmup"_s + "warmup_finished"_e = "running"_s
            , "running"_s + on_entry<_> / [this](){ owner_.enter_running(); }
            , "running"_s + on_exit<_> / [this](){ owner_.leave_running(); }
            , "running"_s + "run_button"_e = "stopped"_s
            , "running"_s + "set_mode_stopped"_e = "stopped"_s
            , "stopped"_s + "cleaning_button"_e = "cleaning"_s
            , "stopped"_s + "set_mode_cleaning"_e = "cleaning"_s
            , "cleaning"_s + on_entry<_> / [this](){ owner_.enter_cleaning(); }
            , "cleaning"_s + on_exit<_> / [this](){ owner_.leave_cleaning(); }
            , "cleaning"_s + "cleaning_button"_e = "stopped"_s
            , "cleaning"_s + "set_mode_stopped"_e = "stopped"_s
            , "stopped"_s + "emergency_on"_e = "emergency"_s
            , "running"_s + "emergency_on"_e = "emergency"_s
            , "emergency"_s + on_entry<_> / [this](){ owner_.enter_emergency(); }
            , "emergency"_s + on_exit<_> / [this](){ owner_.leave_emergency(); }
            , "emergency"_s + "emergency_off"_e = "stopped"_s
            , "stopped"_s + "fault_on"_e = "fault"_s
            , "stopped"_s + "set_mode_fault"_e = "fault"_s
            , "running"_s + "fault_on"_e = "fault"_s
            , "running"_s + "set_mode_fault"_e = "fault"_s
            , "fault"_s + on_entry<_> / [this](){ owner_.enter_fault(); }
            , "fault"_s + on_exit<_> / [this](){ owner_.leave_fault(); }
            , "fault"_s + "fault_off"_e = "stopped"_s
            , "fault"_s + "set_mode_stopped"_e = "stopped"_s
            , "stopped"_s + "maintenance_button"_e = "maintenance"_s
            , "stopped"_s + "set_mode_maintenance"_e = "maintenance"_s
            , "maintenance"_s + on_entry<_> / [this](){ owner_.enter_maintenance(); }
            , "maintenance"_s + on_exit<_> / [this](){ owner_.leave_maintenance(); }
            , "maintenance"_s + "maintenance_button"_e = "stopped"_s
            , "maintenance"_s + "set_mode_stopped"_e = "stopped"_s
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }

private:
  tfc::operation::state_machine& owner_;
};

}  // namespace detail

state_machine::state_machine(boost::asio::io_context& ctx)
    : stopped_{ ctx, "stopped" }, warmup_{ ctx, "warmup" }, running_{ ctx, "running" }, cleaning_{ ctx, "cleaning" },
      mode_{ ctx, "mode" }, mode_str_{ ctx, "mode" }, run_button_{ ctx, "run_button",
                                                                   std::bind_front(&state_machine::running_new_state,
                                                                                   this) },
      cleaning_button_{ ctx, "cleaning_button", std::bind_front(&state_machine::cleaning_new_state, this) },
      maintenance_button_{ ctx, "maintenance_button", std::bind_front(&state_machine::maintenance_new_state, this) },
      logger_{ "state_machine" }, states_{ std::make_shared<boost::sml::sm<detail::state_machine>>(
                                      detail::state_machine{ *this }) } {}

auto operation::state_machine::set_mode(tfc::operation::mode_e new_mode) -> std::error_code {
  using boost::sml::operator""_e;
  logger_.trace("Got new mode: {}", tfc::operation::mode_e_str(new_mode));

  using enum tfc::operation::mode_e;

  bool handled{};
  switch (new_mode) {
    case unknown:
      break;
    case stopped:
      handled = states_->process_event("set_mode_stopped"_e);
      break;
    case starting:  // todo
    case running:
    case stopping:
      handled = states_->process_event("set_mode_running"_e);
      break;
    case cleaning:
      handled = states_->process_event("set_mode_cleaning"_e);
      break;
    case emergency:
      handled = states_->process_event("set_mode_emergency"_e);
      break;
    case fault:
      handled = states_->process_event("set_mode_fault"_e);
      break;
    case maintenance:
      handled = states_->process_event("set_mode_maintenance"_e);
      break;
  }

  if (handled) {
    current_mode_ = new_mode;
  } else {
    return std::make_error_code(std::errc::invalid_argument);
  }
  return {};
}

void state_machine::enter_stopped() {
  stopped_.send(true);
}
void state_machine::leave_stopped() {
  stopped_.send(false);
}
void state_machine::enter_warmup() {
  warmup_.send(true);
}
void state_machine::leave_warmup() {
  warmup_.send(false);
}
void state_machine::enter_running() {
  running_.send(true);
}
void state_machine::leave_running() {
  running_.send(false);
}
void state_machine::enter_cleaning() {
  cleaning_.send(true);
}
void state_machine::leave_cleaning() {
  cleaning_.send(false);
}
void state_machine::enter_emergency() {}
void state_machine::leave_emergency() {}
void state_machine::enter_fault() {}
void state_machine::leave_fault() {}
void state_machine::enter_maintenance() {}
void state_machine::leave_maintenance() {}

void state_machine::running_new_state(bool new_state) {
  if (new_state) {
    set_mode(tfc::operation::mode_e::running);  // todo config
  }
}

void state_machine::cleaning_new_state(bool new_state) {
  if (new_state) {
    set_mode(tfc::operation::mode_e::cleaning);
  }
}

void state_machine::maintenance_new_state(bool new_state) {
  if (new_state) {
    set_mode(tfc::operation::mode_e::maintenance);
  }
}
}  // namespace tfc::operation
