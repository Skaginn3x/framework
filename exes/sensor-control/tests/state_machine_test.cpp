#include <gmock/gmock.h>
#include <boost/sml.hpp>
#include <boost/ut.hpp>

#include <tfc/progbase.hpp>
#include "../inc/state_machine.hpp"

struct the_owner {
  MOCK_METHOD((void), enter_idle, (), ());
  MOCK_METHOD((void), leave_idle, (), ());
  MOCK_METHOD((void), enter_awaiting_discharge, (), ());
  MOCK_METHOD((void), leave_awaiting_discharge, (), ());
  MOCK_METHOD((void), enter_awaiting_sensor, (), ());
  MOCK_METHOD((void), leave_awaiting_sensor, (), ());
  MOCK_METHOD((void), enter_discharging, (), ());
  MOCK_METHOD((void), leave_discharging, (), ());
  MOCK_METHOD((void), enter_discharge_delayed, (), ());
  MOCK_METHOD((void), leave_discharge_delayed, (), ());
  MOCK_METHOD((bool), using_discharge_delay, (), (const noexcept));
  MOCK_METHOD((void), save_time_left, (), ());
};

namespace sml = boost::sml;
namespace ut = boost::ut;
namespace events = tfc::sensor::control::events;
namespace states = tfc::sensor::control::states;
using sml::state;
using ut::operator""_test;
using ut::operator|;
using tfc::sensor::control::state_machine;

struct test_instance {
  testing::NiceMock<the_owner> owner{};
  sml::sm<state_machine<the_owner>, sml::testing> sm{ static_cast<the_owner&>(owner) };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  "idle -> awaiting_release"_test = [] {
    [[maybe_unused]] test_instance instance;
    instance.sm.set_current_states(state<states::idle>);
    EXPECT_CALL(instance.owner, leave_idle());
    EXPECT_CALL(instance.owner, enter_awaiting_discharge());
    instance.sm.process_event(events::sensor_active{});
    ut::expect(instance.sm.is(state<states::awaiting_discharge>));
  };

  "awaiting_sensor -> awaiting_release"_test = [] {
    test_instance instance;
    instance.sm.set_current_states(state<states::awaiting_sensor>);
    EXPECT_CALL(instance.owner, leave_awaiting_sensor());
    EXPECT_CALL(instance.owner, enter_awaiting_discharge());
    instance.sm.process_event(events::sensor_active{});
    ut::expect(instance.sm.is(state<states::awaiting_discharge>));
  };

  "idle -> awaiting_sensor"_test = [] {
    test_instance instance;
    instance.sm.set_current_states(state<states::idle>);
    EXPECT_CALL(instance.owner, leave_idle());
    EXPECT_CALL(instance.owner, enter_awaiting_sensor());
    instance.sm.process_event(events::new_info{});
    ut::expect(instance.sm.is(state<states::awaiting_sensor>));
  };

  "awaiting_discharge -> discharging"_test = [] {
    test_instance instance;
    instance.sm.set_current_states(state<states::awaiting_discharge>);
    EXPECT_CALL(instance.owner, leave_awaiting_discharge());
    EXPECT_CALL(instance.owner, enter_discharging());
    instance.sm.process_event(events::discharge{});
    ut::expect(instance.sm.is(state<states::discharging>));
  };

  "discharging -> idle"_test = [] {
    test_instance instance;
    instance.sm.set_current_states(state<states::discharging>);
    EXPECT_CALL(instance.owner, leave_discharging());
    EXPECT_CALL(instance.owner, enter_idle());
    instance.sm.process_event(events::sensor_inactive{});
    ut::expect(instance.sm.is(state<states::idle>));
  };

  "discharging -> delay discharging"_test = [] {
    test_instance instance;
    instance.sm.set_current_states(state<states::discharging>);
    ON_CALL(instance.owner, using_discharge_delay()).WillByDefault(testing::Return(true));
    EXPECT_CALL(instance.owner, leave_discharging());
    EXPECT_CALL(instance.owner, enter_discharge_delayed());
    instance.sm.process_event(events::sensor_inactive{});
    ut::expect(instance.sm.is(state<states::discharge_delayed>));
  };

  "delay discharging -> idle"_test = [] {
    test_instance instance;
    instance.sm.set_current_states(state<states::discharge_delayed>);
    EXPECT_CALL(instance.owner, leave_discharge_delayed());
    EXPECT_CALL(instance.owner, enter_idle());
    instance.sm.process_event(events::complete{});
    ut::expect(instance.sm.is(state<states::idle>));
  };

  "any -> stopped"_test =
      [](auto const& from_state) {
        test_instance instance;
        instance.sm.set_current_states(from_state);
        instance.sm.process_event(events::stop{});
        ut::expect(instance.sm.is(state<states::stopped>));
      } |
      std::make_tuple(state<states::idle>, state<states::awaiting_discharge>, state<states::awaiting_sensor>,
                      state<states::discharging>, state<states::discharge_delayed>);

  "stopped -> idle"_test = []() {
    test_instance instance;
    instance.sm.set_current_states(state<states::stopped>);
    EXPECT_CALL(instance.owner, enter_idle());
    instance.sm.process_event(events::start{});
    ut::expect(instance.sm.is(state<states::idle>));
  };

  return EXIT_SUCCESS;
}
