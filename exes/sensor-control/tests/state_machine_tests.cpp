#include <boost/ut.hpp>
#include <boost/sml.hpp>
#include <gmock/gmock.h>

#include <tfc/progbase.hpp>
#include "state_machine.hpp"

struct the_owner {
  MOCK_METHOD((void), enter_idle, (), ());
  MOCK_METHOD((void), leave_idle, (), ());
  MOCK_METHOD((void), enter_awaiting_release, (), ());
  MOCK_METHOD((void), leave_awaiting_release, (), ());
  MOCK_METHOD((void), enter_awaiting_sensor, (), ());
  MOCK_METHOD((void), leave_awaiting_sensor, (), ());
};

namespace sml = boost::sml;
namespace ut = boost::ut;
namespace events = tfc::sensor::control::events;
using boost::ut::operator""_test;
using boost::sml::literals::operator""_s;
using tfc::sensor::control::state_machine;


struct test_instance {
  testing::NiceMock<the_owner> owner{};
  sml::sm<state_machine<the_owner>, sml::testing> sm{ state_machine<the_owner>{ owner } };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  "idle -> awaiting_release"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("idle"_s);
    EXPECT_CALL(instance.owner, leave_idle());
    EXPECT_CALL(instance.owner, enter_awaiting_release());
    instance.sm.process_event(events::sensor_active{});
    ut::expect(instance.sm.is("awaiting_release"_s));
  };

  "awaiting_sensor -> awaiting_release"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("awaiting_sensor"_s);
    EXPECT_CALL(instance.owner, leave_awaiting_sensor());
    EXPECT_CALL(instance.owner, enter_awaiting_release());
    instance.sm.process_event(events::sensor_active{});
    ut::expect(instance.sm.is("awaiting_release"_s));
  };

  "idle -> awaiting_sensor"_test = [] {
    test_instance instance;
    instance.sm.set_current_states("idle"_s);
    EXPECT_CALL(instance.owner, leave_idle());
    EXPECT_CALL(instance.owner, enter_awaiting_sensor());
    instance.sm.process_event(events::new_info{});
    ut::expect(instance.sm.is("awaiting_sensor"_s));
  };


  return EXIT_SUCCESS;
}
