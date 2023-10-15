#include <gmock/gmock.h>
#include <boost/sml.hpp>
#include <boost/ut.hpp>

#include <tfc/progbase.hpp>
#include "state_machine.hpp"

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
  MOCK_METHOD((bool), using_discharge_timer, (), (const noexcept));
};

namespace sml = boost::sml;
namespace ut = boost::ut;
namespace events = tfc::sensor::control::events;
using boost::ut::operator""_test;
using boost::sml::literals::operator""_s;
using tfc::sensor::control::state_machine;

struct test_instance {
  testing::NiceMock<the_owner> owner{};
  state_machine<the_owner> sm_owner{ owner };
  sml::sm<state_machine<the_owner>, sml::testing> sm{ sm_owner };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock();

  "idle -> awaiting_release"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("idle"_s);
    EXPECT_CALL(instance.owner, leave_idle());
    EXPECT_CALL(instance.owner, enter_awaiting_discharge());
    instance.sm.process_event(events::sensor_active{});
    ut::expect(instance.sm.is("awaiting_discharge"_s));
  };

  "awaiting_sensor -> awaiting_release"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("awaiting_sensor"_s);
    EXPECT_CALL(instance.owner, leave_awaiting_sensor());
    EXPECT_CALL(instance.owner, enter_awaiting_discharge());
    instance.sm.process_event(events::sensor_active{});
    ut::expect(instance.sm.is("awaiting_discharge"_s));
  };

  "idle -> awaiting_sensor"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("idle"_s);
    EXPECT_CALL(instance.owner, leave_idle());
    EXPECT_CALL(instance.owner, enter_awaiting_sensor());
    instance.sm.process_event(events::new_info{});
    ut::expect(instance.sm.is("awaiting_sensor"_s));
  };

  "awaiting_discharge -> discharging"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("awaiting_discharge"_s);
    EXPECT_CALL(instance.owner, leave_awaiting_discharge());
    EXPECT_CALL(instance.owner, enter_discharging());
    instance.sm.process_event(events::discharge{});
    ut::expect(instance.sm.is("discharging"_s));
  };

  "discharging -> idle"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("discharging"_s);
    EXPECT_CALL(instance.owner, leave_discharging());
    EXPECT_CALL(instance.owner, enter_idle());
    instance.sm.process_event(events::complete{});
    ut::expect(instance.sm.is("idle"_s));
  };

  "discharging -> delay discharging"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("discharging"_s);
    EXPECT_CALL(instance.owner, leave_discharging());
    EXPECT_CALL(instance.owner, enter_discharge_delayed());
    instance.sm.process_event(events::sensor_inactive{});
    ut::expect(instance.sm.is("discharge_delayed"_s));
  };

  "delay discharging -> idle"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("discharge_delayed"_s);
    EXPECT_CALL(instance.owner, leave_discharge_delayed());
    EXPECT_CALL(instance.owner, enter_idle());
    instance.sm.process_event(events::complete{});
    ut::expect(instance.sm.is("idle"_s));
  };

  return EXIT_SUCCESS;
}
