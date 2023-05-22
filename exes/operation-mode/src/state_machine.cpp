#include "state_machine.hpp"
#include <boost/asio.hpp>
#include <tfc/stx/glaze_meta.hpp>

namespace tfc::operation {

namespace detail {

using boost::sml::literals::operator""_e;
using boost::sml::literals::operator""_s;

namespace events {
struct set_starting {};
struct run_button {};
struct starting_timeout {};
struct starting_finished {};
struct set_stopped {};
struct stopping_timeout {};
struct stopping_finished {};
struct cleaning_button {};
struct set_cleaning {};
struct set_emergency {};
struct emergency_on {};
struct emergency_off {};
struct fault_on {};
struct set_fault {};
struct fault_off {};
struct maintenance_button {};
struct set_maintenance {};
}  // namespace events

struct state_machine {
  explicit state_machine(tfc::operation::state_machine& owner) : owner_{ owner } {}

  auto operator()() {
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::on_entry;
    using boost::sml::on_exit;

    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
            * "stopped"_s + on_entry<_> / [this](){ owner_.enter_stopped(); }
            , "stopped"_s + on_exit<_> / [this](){ owner_.leave_stopped(); }
            , "stopped"_s + event<events::set_starting> = "starting"_s
            , "stopped"_s + event<events::run_button> = "starting"_s
            , "starting"_s + on_entry<_> / [this](){ owner_.enter_starting(); }
            , "starting"_s + on_exit<_> / [this](){ owner_.leave_starting(); }
            , "starting"_s + event<events::starting_timeout> = "running"_s
            , "starting"_s + event<events::starting_finished> = "running"_s
            , "running"_s + on_entry<_> / [this](){ owner_.enter_running(); }
            , "running"_s + on_exit<_> / [this](){ owner_.leave_running(); }
            , "running"_s + event<events::run_button> = "stopping"_s
            , "running"_s + event<events::set_stopped> = "stopping"_s
            , "stopping"_s + on_entry<_> / [this](){ owner_.enter_stopping(); }
            , "stopping"_s + on_exit<_> / [this](){ owner_.leave_stopping(); }
            , "stopping"_s + event<events::stopping_timeout> = "stopped"_s
            , "stopping"_s + event<events::stopping_finished> = "stopped"_s
            , "stopped"_s + event<events::cleaning_button> = "cleaning"_s
            , "stopped"_s + event<events::set_cleaning> = "cleaning"_s
            , "cleaning"_s + on_entry<_> / [this](){ owner_.enter_cleaning(); }
            , "cleaning"_s + on_exit<_> / [this](){ owner_.leave_cleaning(); }
            , "cleaning"_s + event<events::cleaning_button> = "stopped"_s
            , "cleaning"_s + event<events::set_stopped> = "stopped"_s
            , "stopped"_s + event<events::set_emergency> = "emergency"_s
            , "running"_s + event<events::set_emergency> = "emergency"_s
            , "cleaning"_s + event<events::set_emergency> = "emergency"_s
            , "stopped"_s + event<events::emergency_on> = "emergency"_s
            , "running"_s + event<events::emergency_on> = "emergency"_s
            , "cleaning"_s + event<events::emergency_on> = "emergency"_s
            , "emergency"_s + on_entry<_> / [this](){ owner_.enter_emergency(); }
            , "emergency"_s + on_exit<_> / [this](){ owner_.leave_emergency(); }
            , "emergency"_s + event<events::emergency_off> = "stopped"_s
            , "stopped"_s + event<events::fault_on> = "fault"_s
            , "stopped"_s + event<events::set_fault> = "fault"_s
            , "running"_s + event<events::fault_on> = "fault"_s
            , "running"_s + event<events::set_fault> = "fault"_s
            , "fault"_s + on_entry<_> / [this](){ owner_.enter_fault(); }
            , "fault"_s + on_exit<_> / [this](){ owner_.leave_fault(); }
            , "fault"_s + event<events::fault_off> = "stopped"_s
            , "fault"_s + event<events::set_stopped> = "stopped"_s
            , "stopped"_s + event<events::maintenance_button> = "maintenance"_s
            , "stopped"_s + event<events::set_maintenance> = "maintenance"_s
            , "maintenance"_s + on_entry<_> / [this](){ owner_.enter_maintenance(); }
            , "maintenance"_s + on_exit<_> / [this](){ owner_.leave_maintenance(); }
            , "maintenance"_s + event<events::maintenance_button> = "stopped"_s
            , "maintenance"_s + event<events::set_stopped> = "stopped"_s
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
    : stopped_{ ctx, "stopped" }, starting_{ ctx, "starting" }, running_{ ctx, "running" }, stopping_{ ctx, "stopping" },
      cleaning_{ ctx, "cleaning" }, mode_{ ctx, "mode" }, mode_str_{ ctx, "mode" },
      run_button_{ ctx, "run_button", std::bind_front(&state_machine::running_new_state, this) },
      cleaning_button_{ ctx, "cleaning_button", std::bind_front(&state_machine::cleaning_new_state, this) },
      maintenance_button_{ ctx, "maintenance_button", std::bind_front(&state_machine::maintenance_new_state, this) },
      logger_{ "state_machine" },
      config_{ ctx, "state_machine" },
      states_{ std::make_shared<boost::sml::sm<detail::state_machine, boost::sml::logger<detail::my_logger>>>(
          detail::state_machine{ *this },
          detail::my_logger{}) } {}

auto operation::state_machine::set_mode(tfc::operation::mode_e new_mode) -> std::error_code {
  logger_.trace("Got new mode: {}", tfc::operation::mode_e_str(new_mode));

  bool handled{};
  switch (new_mode) {
    using enum tfc::operation::mode_e;
    case unknown:
      break;
    case stopping:
    case stopped:
      handled = states_->process_event(detail::events::set_stopped{});
      break;
    case starting:
    case running:
      handled = states_->process_event(detail::events::set_starting{});
      break;
    case cleaning:
      handled = states_->process_event(detail::events::set_cleaning{});
      break;
    case emergency:
      handled = states_->process_event(detail::events::set_emergency{});
      break;
    case fault:
      handled = states_->process_event(detail::events::set_fault{});
      break;
    case maintenance:
      handled = states_->process_event(detail::events::set_maintenance{});
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
void state_machine::enter_starting() {
  starting_.send(true);
}
void state_machine::leave_starting() {
  starting_.send(false);
}
void state_machine::enter_running() {
  running_.send(true);
}
void state_machine::leave_running() {
  running_.send(false);
}
void state_machine::enter_stopping() {
  stopping_.send(true);
}
void state_machine::leave_stopping() {
  stopping_.send(false);
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
