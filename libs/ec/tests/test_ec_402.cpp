#include <iostream>

#include <boost/ut.hpp>

#include <tfc/cia/402.hpp>

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  using tfc::ec::cia_402::commands_e;
  using tfc::ec::cia_402::parse_state;
  using tfc::ec::cia_402::states_e;
  using tfc::ec::cia_402::status_word;
  using tfc::ec::cia_402::transition;

  "parse state test"_test = []() {
    expect(parse_state(status_word::from_uint(0xff40U)) == states_e::switch_on_disabled);
    expect(parse_state(status_word::from_uint(0xff50U)) == states_e::switch_on_disabled);
    expect(parse_state(status_word::from_uint(0xff21U)) == states_e::ready_to_switch_on);
    expect(parse_state(status_word::from_uint(0xff31U)) == states_e::ready_to_switch_on);
    expect(parse_state(status_word::from_uint(0xfff8U)) == states_e::fault);
    expect(parse_state(status_word::from_uint(0x0008U)) == states_e::fault);
    expect(parse_state(status_word::from_uint(0xff33U)) == states_e::switched_on);
    expect(parse_state(status_word::from_uint(0xff37U)) == states_e::operation_enabled);
    expect(parse_state(status_word::from_uint(0xff17U)) == states_e::quick_stop_active);
  };

  "transition_to_operation"_test = []() {
    expect(transition(states_e::switch_on_disabled, false) == commands_e::shutdown);
    expect(transition(states_e::ready_to_switch_on, false) == commands_e::enable_operation);
    expect(transition(states_e::switched_on, false) == commands_e::enable_operation);
    expect(transition(states_e::fault, false) == commands_e::fault_reset);
    expect(transition(states_e::operation_enabled, false) == commands_e::enable_operation);
  };
}

namespace compile_tests {
using tfc::ec::cia_402::states_e;
using tfc::ec::cia_402::status_word;

static_assert(states_e::not_ready_to_switch_on == static_cast<states_e>(status_word{ .state_quick_stop = 1 }));
static_assert(states_e::not_ready_to_switch_on == static_cast<states_e>(status_word{ .voltage_enabled = 1 }));
static_assert(states_e::switch_on_disabled == static_cast<states_e>(status_word{ .state_switch_on_disabled = 1 }));
static_assert(states_e::switch_on_disabled ==
              static_cast<states_e>(status_word{ .state_quick_stop = 1, .state_switch_on_disabled = 1 }));
static_assert(states_e::switch_on_disabled ==
              static_cast<states_e>(status_word{ .voltage_enabled = 1, .state_switch_on_disabled = 1 }));
static_assert(states_e::switch_on_disabled == static_cast<states_e>(status_word{ .voltage_enabled = 1,
                                                                                 .state_quick_stop = 1,
                                                                                 .state_switch_on_disabled = 1 }));
static_assert(states_e::ready_to_switch_on ==
              static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1, .state_quick_stop = 1 }));
static_assert(states_e::ready_to_switch_on == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                                 .voltage_enabled = 1,
                                                                                 .state_quick_stop = 1 }));
static_assert(states_e::switched_on == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                          .state_switched_on = 1,
                                                                          .voltage_enabled = 1,
                                                                          .state_quick_stop = 1 }));
static_assert(states_e::operation_enabled == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                                .state_switched_on = 1,
                                                                                .state_operation_enabled = 1,
                                                                                .voltage_enabled = 1,
                                                                                .state_quick_stop = 1 }));
static_assert(states_e::quick_stop_active == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                                .state_switched_on = 1,
                                                                                .state_operation_enabled = 1,
                                                                                .voltage_enabled = 1,
                                                                                .state_quick_stop = 0 }));
static_assert(states_e::fault_reaction_active == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                                    .state_switched_on = 1,
                                                                                    .state_operation_enabled = 1,
                                                                                    .state_fault = 1 }));
static_assert(states_e::fault_reaction_active == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                                    .state_switched_on = 1,
                                                                                    .state_operation_enabled = 1,
                                                                                    .state_fault = 1,
                                                                                    .voltage_enabled = 1 }));
static_assert(states_e::fault_reaction_active == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                                    .state_switched_on = 1,
                                                                                    .state_operation_enabled = 1,
                                                                                    .state_fault = 1,
                                                                                    .state_quick_stop = 1 }));
static_assert(states_e::fault_reaction_active == static_cast<states_e>(status_word{ .state_ready_to_switch_on = 1,
                                                                                    .state_switched_on = 1,
                                                                                    .state_operation_enabled = 1,
                                                                                    .state_fault = 1,
                                                                                    .voltage_enabled = 1,
                                                                                    .state_quick_stop = 1 }));
static_assert(states_e::fault == static_cast<states_e>(status_word{ .state_fault = 1 }));
static_assert(states_e::fault == static_cast<states_e>(status_word{ .state_fault = 1, .voltage_enabled = 1 }));
static_assert(states_e::fault == static_cast<states_e>(status_word{ .state_fault = 1, .state_quick_stop = 1 }));
static_assert(states_e::fault ==
              static_cast<states_e>(status_word{ .state_fault = 1, .voltage_enabled = 1, .state_quick_stop = 1 }));

}  // namespace compile_tests
