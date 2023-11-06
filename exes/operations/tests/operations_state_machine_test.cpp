#include <gmock/gmock.h>
#include <boost/sml.hpp>
#include <boost/ut.hpp>

#include <tfc/operation_mode/common.hpp>
#include "details/state_machine.hpp"

import tfc.base;
import fmt;

struct the_owner {
  MOCK_METHOD((void), enter_stopped, (), ());
  MOCK_METHOD((void), leave_stopped, (), ());
  MOCK_METHOD((void), enter_starting, (), ());
  MOCK_METHOD((void), leave_starting, (), ());
  MOCK_METHOD((void), enter_running, (), ());
  MOCK_METHOD((void), leave_running, (), ());
  MOCK_METHOD((void), enter_stopping, (), ());
  MOCK_METHOD((void), leave_stopping, (), ());
  MOCK_METHOD((void), enter_cleaning, (), ());
  MOCK_METHOD((void), leave_cleaning, (), ());
  MOCK_METHOD((void), enter_emergency, (), ());
  MOCK_METHOD((void), leave_emergency, (), ());
  MOCK_METHOD((void), enter_fault, (), ());
  MOCK_METHOD((void), leave_fault, (), ());
  MOCK_METHOD((void), enter_maintenance, (), ());
  MOCK_METHOD((void), leave_maintenance, (), ());
  MOCK_METHOD((void), transition, (tfc::operation::mode_e, tfc::operation::mode_e), ());
};

namespace sml = boost::sml;
namespace ut = boost::ut;
namespace events = tfc::operation::detail::events;
namespace states = tfc::operation::detail::states;
using boost::ut::operator""_test;
using boost::ut::operator|;
using boost::sml::state;
using tfc::operation::detail::state_machine;

struct test_instance {
  testing::NiceMock<the_owner> owner{};
  sml::sm<state_machine<the_owner>, sml::testing> sm{ state_machine<the_owner>{ owner } };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  using enum tfc::operation::mode_e;

  "init -> stopped"_test = [] {
    test_instance instance;
    instance.sm.set_current_states(state<states::init>);
    EXPECT_CALL(instance.owner, enter_stopped());
    EXPECT_CALL(instance.owner, transition(stopped, unknown));
    instance.sm.process_event(events::set_stopped{});
    ut::expect(instance.sm.is(state<states::stopped>));
  };

  "cleaning -> stopped"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::cleaning>);
    EXPECT_CALL(instance.owner, leave_cleaning());
    EXPECT_CALL(instance.owner, enter_stopped());
    EXPECT_CALL(instance.owner, transition(stopped, cleaning));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::stopped>));
  } | std::tuple<events::cleaning_button, events::set_stopped>{};

  "stopping -> stopped"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::stopping>);
    EXPECT_CALL(instance.owner, leave_stopping());
    EXPECT_CALL(instance.owner, enter_stopped());
    EXPECT_CALL(instance.owner, transition(stopped, stopping));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::stopped>));
  } | std::tuple<events::stopping_timeout, events::stopping_finished>{};

  "fault -> stopped"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::fault>);
    EXPECT_CALL(instance.owner, leave_fault());
    EXPECT_CALL(instance.owner, enter_stopped());
    EXPECT_CALL(instance.owner, transition(stopped, fault));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::stopped>));
  } | std::tuple<events::fault_off, events::set_stopped>{};

  "maintenance -> stopped"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::maintenance>);
    EXPECT_CALL(instance.owner, leave_maintenance());
    EXPECT_CALL(instance.owner, enter_stopped());
    EXPECT_CALL(instance.owner, transition(stopped, maintenance));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::stopped>));
  } | std::tuple<events::maintenance_button, events::set_stopped>{};

  "stopped -> starting"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::stopped>);
    EXPECT_CALL(instance.owner, leave_stopped());
    EXPECT_CALL(instance.owner, enter_starting());
    EXPECT_CALL(instance.owner, transition(starting, stopped));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::starting>));
  } | std::tuple<events::set_starting, events::run_button>{};

  "starting -> running"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::starting>);
    EXPECT_CALL(instance.owner, leave_starting());
    EXPECT_CALL(instance.owner, enter_running());
    EXPECT_CALL(instance.owner, transition(running, starting));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::running>));
  } | std::tuple<events::starting_timeout, events::starting_finished>{};

  "running -> stopping"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::running>);
    EXPECT_CALL(instance.owner, leave_running());
    EXPECT_CALL(instance.owner, enter_stopping());
    EXPECT_CALL(instance.owner, transition(stopping, running));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::stopping>));
  } | std::tuple<events::run_button, events::set_stopped>{};

  "stopped -> cleaning"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::stopped>);
    EXPECT_CALL(instance.owner, leave_stopped());
    EXPECT_CALL(instance.owner, enter_cleaning());
    EXPECT_CALL(instance.owner, transition(cleaning, stopped));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::cleaning>));
  } | std::tuple<events::cleaning_button, events::set_cleaning>{};

  "running -> fault"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::running>);
    EXPECT_CALL(instance.owner, leave_running());
    EXPECT_CALL(instance.owner, enter_fault());
    EXPECT_CALL(instance.owner, transition(fault, running));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::fault>));
  } | std::tuple<events::fault_on, events::set_fault>{};

  "stopped -> maintenance"_test = []<typename event_t> {
    test_instance instance;
    instance.sm.set_current_states(state<states::stopped>);
    EXPECT_CALL(instance.owner, leave_stopped());
    EXPECT_CALL(instance.owner, enter_maintenance());
    EXPECT_CALL(instance.owner, transition(maintenance, stopped));
    instance.sm.process_event(event_t{});
    ut::expect(instance.sm.is(state<states::maintenance>));
  } | std::tuple<events::maintenance_button, events::set_maintenance>{};

  "all -> emergency"_test =
      []<typename state_t>(state_t const& state) {
        ut::test(fmt::format("{} -> emergency", state_t::type::name)) = [&state]<typename event_t> {
          test_instance instance;
          instance.sm.set_current_states(state);
          using enum tfc::operation::mode_e;
          if constexpr (state_t::type::enum_v == stopped) {
            EXPECT_CALL(instance.owner, leave_stopped());
          } else if constexpr (state_t::type::enum_v == starting) {
            EXPECT_CALL(instance.owner, leave_starting());
          } else if constexpr (state_t::type::enum_v == stopping) {
            EXPECT_CALL(instance.owner, leave_stopping());
          } else if constexpr (state_t::type::enum_v == running) {
            EXPECT_CALL(instance.owner, leave_running());
          } else if constexpr (state_t::type::enum_v == cleaning) {
            EXPECT_CALL(instance.owner, leave_cleaning());
          } else if constexpr (state_t::type::enum_v == fault) {
            EXPECT_CALL(instance.owner, leave_fault());
          } else if constexpr (state_t::type::enum_v == maintenance) {
            EXPECT_CALL(instance.owner, leave_maintenance());
          } else {
            []<bool flag = false>() {
              static_assert(flag, "Unhandled state");
            }
            ();
          }
          EXPECT_CALL(instance.owner, enter_emergency());
          EXPECT_CALL(instance.owner, transition(emergency, state_t::type::enum_v));
          instance.sm.process_event(event_t{});
          ut::expect(instance.sm.is(::state<states::emergency>));
        } | std::tuple<events::set_emergency, events::emergency_on>{};
      } |
      std::make_tuple(state<states::stopped>, state<states::stopping>, state<states::starting>, state<states::running>,
                      state<states::cleaning>, state<states::fault>, state<states::maintenance>);

  return EXIT_SUCCESS;
}
