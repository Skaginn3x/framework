#include <iostream>

#include <boost/ut.hpp>

#include <tfc/cia/402.hpp>

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  using tfc::ec::cia_402::commands_e;
  using tfc::ec::cia_402::parse_state;
  using tfc::ec::cia_402::states_e;
  using tfc::ec::cia_402::transition;

  "parse state test"_test = []() {
    expect(parse_state(0xff40) == states_e::switch_on_disabled);
    expect(parse_state(0xff50) == states_e::switch_on_disabled);
    expect(parse_state(0xff21) == states_e::ready_to_switch_on);
    expect(parse_state(0xff31) == states_e::ready_to_switch_on);
    expect(parse_state(0xfff8) == states_e::fault);
    expect(parse_state(0x0008) == states_e::fault);
    expect(parse_state(0xff33) == states_e::switched_on);
    expect(parse_state(0xff37) == states_e::operation_enabled);
    expect(parse_state(0xff17) == states_e::quick_stop_active);
  };

  "transition_to_operation"_test = []() {
    expect(transition(states_e::switch_on_disabled, false) == commands_e::shutdown);
    expect(transition(states_e::ready_to_switch_on, false) == commands_e::enable_operation);
    expect(transition(states_e::switched_on, false) == commands_e::enable_operation);
    expect(transition(states_e::fault, false) == commands_e::fault_reset);
    expect(transition(states_e::operation_enabled, false) == commands_e::enable_operation);
  };
}
