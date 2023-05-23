#include "state_machine.hpp"
#include <boost/asio.hpp>
#include <tfc/stx/glaze_meta.hpp>

namespace tfc::operation {

namespace detail {

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
    using boost::sml::literals::operator""_s;

    using enum tfc::operation::mode_e;
    // todo meta-program this transition boiler plate code
    // if states would declare constexpr of enum this should be doable, btw logger has src and destination state
    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
            * "stopped"_s + on_entry<_> / [this](){ owner_.enter_stopped(); }
            , "stopped"_s + on_exit<_> / [this](){ owner_.leave_stopped(); }
            , "stopped"_s + event<events::set_starting> / [this](){ owner_.transition(starting, stopped); } = "starting"_s
            , "stopped"_s + event<events::run_button> / [this](){ owner_.transition(starting, stopped); } = "starting"_s
            , "starting"_s + on_entry<_> / [this](){ owner_.enter_starting(); }
            , "starting"_s + on_exit<_> / [this](){ owner_.leave_starting(); }
            , "starting"_s + event<events::starting_timeout> / [this](){ owner_.transition(running, starting); } = "running"_s
            , "starting"_s + event<events::starting_finished> / [this](){ owner_.transition(running, starting); } = "running"_s
            , "running"_s + on_entry<_> / [this](){ owner_.enter_running(); }
            , "running"_s + on_exit<_> / [this](){ owner_.leave_running(); }
            , "running"_s + event<events::run_button> / [this](){ owner_.transition(stopping, running); } = "stopping"_s
            , "running"_s + event<events::set_stopped> / [this](){ owner_.transition(stopping, running); } = "stopping"_s
            , "stopping"_s + on_entry<_> / [this](){ owner_.enter_stopping(); }
            , "stopping"_s + on_exit<_> / [this](){ owner_.leave_stopping(); }
            , "stopping"_s + event<events::stopping_timeout> / [this](){ owner_.transition(stopped, stopping); } = "stopped"_s
            , "stopping"_s + event<events::stopping_finished> / [this](){ owner_.transition(stopped, stopping); } = "stopped"_s
            , "stopped"_s + event<events::cleaning_button> / [this](){ owner_.transition(cleaning, stopped); } = "cleaning"_s
            , "stopped"_s + event<events::set_cleaning> / [this](){ owner_.transition(cleaning, stopped); } = "cleaning"_s
            , "cleaning"_s + on_entry<_> / [this](){ owner_.enter_cleaning(); }
            , "cleaning"_s + on_exit<_> / [this](){ owner_.leave_cleaning(); }
            , "cleaning"_s + event<events::cleaning_button> / [this](){ owner_.transition(stopped, cleaning); } = "stopped"_s
            , "cleaning"_s + event<events::set_stopped> / [this](){ owner_.transition(stopped, cleaning); } = "stopped"_s
            , "stopped"_s + event<events::set_emergency> / [this](){ owner_.transition(emergency, stopped); } = "emergency"_s
            , "running"_s + event<events::set_emergency> / [this](){ owner_.transition(emergency, running); } = "emergency"_s
            , "cleaning"_s + event<events::set_emergency> / [this](){ owner_.transition(emergency, cleaning); } = "emergency"_s
            , "stopped"_s + event<events::emergency_on> / [this](){ owner_.transition(emergency, stopped); } = "emergency"_s
            , "running"_s + event<events::emergency_on> / [this](){ owner_.transition(emergency, running); } = "emergency"_s
            , "cleaning"_s + event<events::emergency_on> / [this](){ owner_.transition(emergency, cleaning); } = "emergency"_s
            , "emergency"_s + on_entry<_> / [this](){ owner_.enter_emergency(); }
            , "emergency"_s + on_exit<_> / [this](){ owner_.leave_emergency(); }
            , "emergency"_s + event<events::emergency_off> / [this](){ owner_.transition(stopped, emergency); } = "stopped"_s
            , "stopped"_s + event<events::fault_on> / [this](){ owner_.transition(fault, stopped); } = "fault"_s
            , "stopped"_s + event<events::set_fault> / [this](){ owner_.transition(fault, stopped); } = "fault"_s
            , "running"_s + event<events::fault_on> / [this](){ owner_.transition(fault, running); } = "fault"_s
            , "running"_s + event<events::set_fault> / [this](){ owner_.transition(fault, running); } = "fault"_s
            , "fault"_s + on_entry<_> / [this](){ owner_.enter_fault(); }
            , "fault"_s + on_exit<_> / [this](){ owner_.leave_fault(); }
            , "fault"_s + event<events::fault_off> / [this](){ owner_.transition(stopped, fault); } = "stopped"_s
            , "fault"_s + event<events::set_stopped> / [this](){ owner_.transition(stopped, fault); } = "stopped"_s
            , "stopped"_s + event<events::maintenance_button> / [this](){ owner_.transition(maintenance, stopped); } = "maintenance"_s
            , "stopped"_s + event<events::set_maintenance> / [this](){ owner_.transition(maintenance, stopped); } = "maintenance"_s
            , "maintenance"_s + on_entry<_> / [this](){ owner_.enter_maintenance(); }
            , "maintenance"_s + on_exit<_> / [this](){ owner_.leave_maintenance(); }
            , "maintenance"_s + event<events::maintenance_button> / [this](){ owner_.transition(stopped, maintenance); } = "stopped"_s
            , "maintenance"_s + event<events::set_stopped> / [this](){ owner_.transition(stopped, maintenance); } = "stopped"_s
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }

private:
  tfc::operation::state_machine& owner_;
};

struct sml_logger {
  template <class SM, class TEvent>
  void log_process_event(const TEvent&) {
    logger_->trace("[{}][process_event] {}\n", boost::sml::aux::get_type_name<SM>(),
                   boost::sml::aux::get_type_name<TEvent>());
  }

  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard&, const TEvent&, bool result) {
    logger_->trace("[{}][guard] {} {} {}\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TGuard>(),
                   boost::sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]"));
  }

  template <class SM, class TAction, class TEvent>
  void log_action(const TAction&, const TEvent&) {
    logger_->trace("[{}][action] {} {}\n", boost::sml::aux::get_type_name<SM>(), boost::sml::aux::get_type_name<TAction>(),
                   boost::sml::aux::get_type_name<TEvent>());
  }

  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {
    logger_->trace("[{}][transition] {} -> {}\n", boost::sml::aux::get_type_name<SM>(), src.c_str(), dst.c_str());
  }

private:
  std::shared_ptr<tfc::logger::logger> logger_{ std::make_shared<tfc::logger::logger>("sml") };
};

}  // namespace detail

state_machine::state_machine(boost::asio::io_context& ctx)
    : stopped_{ ctx, "stopped" }, starting_{ ctx, "starting" }, running_{ ctx, "running" }, stopping_{ ctx, "stopping" },
      cleaning_{ ctx, "cleaning" }, mode_{ ctx, "mode" }, mode_str_{ ctx, "mode" },
      run_button_{ ctx, "run_button", std::bind_front(&state_machine::running_new_state, this) },
      cleaning_button_{ ctx, "cleaning_button", std::bind_front(&state_machine::cleaning_new_state, this) },
      maintenance_button_{ ctx, "maintenance_button", std::bind_front(&state_machine::maintenance_new_state, this) },
      logger_{ "state_machine" }, config_{ ctx, "state_machine" },
      states_{ std::make_shared<boost::sml::sm<detail::state_machine, boost::sml::logger<detail::sml_logger>>>(
          detail::state_machine{ *this },
          detail::sml_logger{}) },
      ctx_{ ctx } {}

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

  if (!handled) {
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
  if (config_->startup_time) {
    auto timer{ std::make_shared<boost::asio::steady_timer>(ctx_) };
    timer->expires_from_now(config_->startup_time.value());
    timer->async_wait([this, timer](std::error_code const& err) {
      if (err) {
        return;
      }
      this->states_->process_event(detail::events::starting_timeout{});
    });
  }
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
  if (config_->stopping_time) {
    auto timer{ std::make_shared<boost::asio::steady_timer>(ctx_) };
    timer->expires_from_now(config_->stopping_time.value());
    timer->async_wait([this, timer](std::error_code const& err) {
      if (err) {
        return;
      }
      this->states_->process_event(detail::events::stopping_timeout{});
    });
  }
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
void state_machine::transition(mode_e new_mode, mode_e old_mode) const {
  if (on_new_state_) {
    std::invoke(on_new_state_, new_mode, old_mode);
  }
}

void state_machine::running_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::run_button{});
  }
}

void state_machine::cleaning_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::cleaning_button{});
  }
}

void state_machine::maintenance_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::maintenance_button{});
  }
}
}  // namespace tfc::operation
