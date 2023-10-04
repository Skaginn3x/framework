#pragma once

#include <boost/sml.hpp>

#include <tfc/operation_mode/common.hpp>

namespace tfc::operation::detail {

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

namespace states {
struct init {
  static constexpr std::string_view name{ "init" };
  static constexpr auto enum_v{ mode_e::unknown };
};
struct stopped {
  static constexpr std::string_view name{ "stopped" };
  static constexpr auto enum_v{ mode_e::stopped };
};
struct starting {
  static constexpr std::string_view name{ "starting" };
  static constexpr auto enum_v{ mode_e::starting };
};
struct running {
  static constexpr std::string_view name{ "running" };
  static constexpr auto enum_v{ mode_e::running };
};
struct stopping {
  static constexpr std::string_view name{ "stopping" };
  static constexpr auto enum_v{ mode_e::stopping };
};
struct cleaning {
  static constexpr std::string_view name{ "cleaning" };
  static constexpr auto enum_v{ mode_e::cleaning };
};
struct emergency {
  static constexpr std::string_view name{ "emergency" };
  static constexpr auto enum_v{ mode_e::emergency };
};
struct fault {
  static constexpr std::string_view name{ "fault" };
  static constexpr auto enum_v{ mode_e::fault };
};
struct maintenance {
  static constexpr std::string_view name{ "maintenance" };
  static constexpr auto enum_v{ mode_e::maintenance };
};
}  // namespace states

template <typename owner_t>
struct state_machine {
  explicit state_machine(owner_t& owner) : owner_{ owner } {}

  auto operator()() {
    using boost::sml::_;
    using boost::sml::event;
    using boost::sml::on_entry;
    using boost::sml::on_exit;
    using boost::sml::state;

    using enum tfc::operation::mode_e;
    // todo meta-program this transition boiler plate code
    // if states would declare constexpr of enum this should be doable, btw logger has src and destination state
    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wused-but-marked-unused) // Todo fix sml.hpp
    auto table = boost::sml::make_transition_table(
            * state<states::init> + event<events::set_stopped> / [this](){ owner_.transition(stopped, unknown); } = state<states::stopped>
            , state<states::stopped> + on_entry<_> / [this](){ owner_.enter_stopped(); }
            , state<states::stopped> + on_exit<_> / [this](){ owner_.leave_stopped(); }
            , state<states::stopped> + event<events::set_starting> / [this](){ owner_.transition(starting, stopped); } = state<states::starting>
            , state<states::stopped> + event<events::run_button> / [this](){ owner_.transition(starting, stopped); } = state<states::starting>
            , state<states::starting> + on_entry<_> / [this](){ owner_.enter_starting(); }
            , state<states::starting> + on_exit<_> / [this](){ owner_.leave_starting(); }
            , state<states::starting> + event<events::starting_timeout> / [this](){ owner_.transition(running, starting); } = state<states::running>
            , state<states::starting> + event<events::starting_finished> / [this](){ owner_.transition(running, starting); } = state<states::running>
            , state<states::running> + on_entry<_> / [this](){ owner_.enter_running(); }
            , state<states::running> + on_exit<_> / [this](){ owner_.leave_running(); }
            , state<states::running> + event<events::run_button> / [this](){ owner_.transition(stopping, running); } = state<states::stopping>
            , state<states::running> + event<events::set_stopped> / [this](){ owner_.transition(stopping, running); } = state<states::stopping>
            , state<states::stopping> + on_entry<_> / [this](){ owner_.enter_stopping(); }
            , state<states::stopping> + on_exit<_> / [this](){ owner_.leave_stopping(); }
            , state<states::stopping> + event<events::stopping_timeout> / [this](){ owner_.transition(stopped, stopping); } = state<states::stopped>
            , state<states::stopping> + event<events::stopping_finished> / [this](){ owner_.transition(stopped, stopping); } = state<states::stopped>
            , state<states::stopped> + event<events::cleaning_button> / [this](){ owner_.transition(cleaning, stopped); } = state<states::cleaning>
            , state<states::stopped> + event<events::set_cleaning> / [this](){ owner_.transition(cleaning, stopped); } = state<states::cleaning>
            , state<states::cleaning> + on_entry<_> / [this](){ owner_.enter_cleaning(); }
            , state<states::cleaning> + on_exit<_> / [this](){ owner_.leave_cleaning(); }
            , state<states::cleaning> + event<events::cleaning_button> / [this](){ owner_.transition(stopped, cleaning); } = state<states::stopped>
            , state<states::cleaning> + event<events::set_stopped> / [this](){ owner_.transition(stopped, cleaning); } = state<states::stopped>

            , state<states::stopped> + event<events::set_emergency> / [this](){ owner_.transition(emergency, stopped); } = state<states::emergency>
            , state<states::stopping> + event<events::set_emergency> / [this](){ owner_.transition(emergency, stopping); } = state<states::emergency>
            , state<states::starting> + event<events::set_emergency> / [this](){ owner_.transition(emergency, starting); } = state<states::emergency>
            , state<states::running> + event<events::set_emergency> / [this](){ owner_.transition(emergency, running); } = state<states::emergency>
            , state<states::cleaning> + event<events::set_emergency> / [this](){ owner_.transition(emergency, cleaning); } = state<states::emergency>
            , state<states::fault> + event<events::set_emergency> / [this](){ owner_.transition(emergency, fault); } = state<states::emergency>
            , state<states::maintenance> + event<events::set_emergency> / [this](){ owner_.transition(emergency, maintenance); } = state<states::emergency>

            , state<states::stopped> + event<events::emergency_on> / [this](){ owner_.transition(emergency, stopped); } = state<states::emergency>
            , state<states::stopping> + event<events::emergency_on> / [this](){ owner_.transition(emergency, stopping); } = state<states::emergency>
            , state<states::starting> + event<events::emergency_on> / [this](){ owner_.transition(emergency, starting); } = state<states::emergency>
            , state<states::running> + event<events::emergency_on> / [this](){ owner_.transition(emergency, running); } = state<states::emergency>
            , state<states::cleaning> + event<events::emergency_on> / [this](){ owner_.transition(emergency, cleaning); } = state<states::emergency>
            , state<states::fault> + event<events::emergency_on> / [this](){ owner_.transition(emergency, fault); } = state<states::emergency>
            , state<states::maintenance> + event<events::emergency_on> / [this](){ owner_.transition(emergency, maintenance); } = state<states::emergency>

            , state<states::emergency> + on_entry<_> / [this](){ owner_.enter_emergency(); }
            , state<states::emergency> + on_exit<_> / [this](){ owner_.leave_emergency(); }
            , state<states::emergency> + event<events::emergency_off> / [this](){ owner_.transition(stopped, emergency); } = state<states::stopped>
            , state<states::stopped> + event<events::fault_on> / [this](){ owner_.transition(fault, stopped); } = state<states::fault>
            , state<states::stopped> + event<events::set_fault> / [this](){ owner_.transition(fault, stopped); } = state<states::fault>
            , state<states::running> + event<events::fault_on> / [this](){ owner_.transition(fault, running); } = state<states::fault>
            , state<states::running> + event<events::set_fault> / [this](){ owner_.transition(fault, running); } = state<states::fault>
            , state<states::fault> + on_entry<_> / [this](){ owner_.enter_fault(); }
            , state<states::fault> + on_exit<_> / [this](){ owner_.leave_fault(); }
            , state<states::fault> + event<events::fault_off> / [this](){ owner_.transition(stopped, fault); } = state<states::stopped>
            , state<states::fault> + event<events::set_stopped> / [this](){ owner_.transition(stopped, fault); } = state<states::stopped>
            , state<states::stopped> + event<events::maintenance_button> / [this](){ owner_.transition(maintenance, stopped); } = state<states::maintenance>
            , state<states::stopped> + event<events::set_maintenance> / [this](){ owner_.transition(maintenance, stopped); } = state<states::maintenance>
            , state<states::maintenance> + on_entry<_> / [this](){ owner_.enter_maintenance(); }
            , state<states::maintenance> + on_exit<_> / [this](){ owner_.leave_maintenance(); }
            , state<states::maintenance> + event<events::maintenance_button> / [this](){ owner_.transition(stopped, maintenance); } = state<states::stopped>
            , state<states::maintenance> + event<events::set_stopped> / [this](){ owner_.transition(stopped, maintenance); } = state<states::stopped>
    );
    PRAGMA_CLANG_WARNING_POP
    // clang-format on
    return table;
  }

private:
  owner_t& owner_;
};

}  // namespace tfc::operation::detail
