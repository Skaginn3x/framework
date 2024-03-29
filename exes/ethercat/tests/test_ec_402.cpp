#include <iostream>

#include <boost/ut.hpp>

#include <tfc/cia/402.hpp>

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  using tfc::ec::cia_402::states_e;
  using tfc::ec::cia_402::status_word;
  using tfc::ec::cia_402::transition;

  "parse state test"_test = []() {
// bitcast is not completely constexpr in clang, memcpy is not constexpr, underlying behaviour of bitcast
#ifndef __clang__
    static_assert(status_word::from_uint(0xff40U).parse_state() == states_e::switch_on_disabled);
    static_assert(status_word::from_uint(0xff50U).parse_state() == states_e::switch_on_disabled);
    static_assert(status_word::from_uint(0xff21U).parse_state() == states_e::ready_to_switch_on);
    static_assert(status_word::from_uint(0xff31U).parse_state() == states_e::ready_to_switch_on);
    static_assert(status_word::from_uint(0xfff8U).parse_state() == states_e::fault);
    static_assert(status_word::from_uint(0x0008U).parse_state() == states_e::fault);
    static_assert(status_word::from_uint(0xff33U).parse_state() == states_e::switched_on);
    static_assert(status_word::from_uint(0xff37U).parse_state() == states_e::operation_enabled);
    static_assert(status_word::from_uint(0xff17U).parse_state() == states_e::quick_stop_active);
#endif
  };

  "transition_to_operation"_test = [] {
    using tfc::ec::cia_402::commands;
    using enum tfc::ec::cia_402::transition_action;
    using enum states_e;
    expect(transition(switch_on_disabled, run) == commands::shutdown());
    expect(transition(ready_to_switch_on, run) == commands::enable_operation());
    expect(transition(switched_on, run) == commands::enable_operation());
    bool const allow_reset_fault{ true };
    expect(transition(fault, run, allow_reset_fault) == commands::fault_reset());
    expect(transition(fault, run) == commands::shutdown());
    expect(transition(operation_enabled, run) == commands::enable_operation());
  };
}

namespace compile_tests {
// using tfc::ec::cia_402::commands_e;
using tfc::ec::cia_402::control_word;
using tfc::ec::cia_402::states_e;
using tfc::ec::cia_402::status_word;

static_assert(states_e::not_ready_to_switch_on == status_word{ .state_quick_stop = 1 }.parse_state());
static_assert(states_e::not_ready_to_switch_on == status_word{ .voltage_enabled = 1 }.parse_state());
static_assert(states_e::switch_on_disabled == status_word{ .state_switch_on_disabled = 1 }.parse_state());
static_assert(states_e::switch_on_disabled ==
              status_word{ .state_quick_stop = 1, .state_switch_on_disabled = 1 }.parse_state());
static_assert(states_e::switch_on_disabled ==
              status_word{ .voltage_enabled = 1, .state_switch_on_disabled = 1 }.parse_state());
static_assert(states_e::switch_on_disabled ==
              status_word{ .voltage_enabled = 1, .state_quick_stop = 1, .state_switch_on_disabled = 1 }.parse_state());
static_assert(states_e::ready_to_switch_on ==
              status_word{ .state_ready_to_switch_on = 1, .state_quick_stop = 1 }.parse_state());
static_assert(states_e::ready_to_switch_on ==
              status_word{ .state_ready_to_switch_on = 1, .voltage_enabled = 1, .state_quick_stop = 1 }.parse_state());
static_assert(states_e::switched_on == status_word{ .state_ready_to_switch_on = 1,
                                                    .state_switched_on = 1,
                                                    .voltage_enabled = 1,
                                                    .state_quick_stop = 1 }
                                           .parse_state());
static_assert(states_e::operation_enabled == status_word{ .state_ready_to_switch_on = 1,
                                                          .state_switched_on = 1,
                                                          .state_operation_enabled = 1,
                                                          .voltage_enabled = 1,
                                                          .state_quick_stop = 1 }
                                                 .parse_state());
static_assert(states_e::quick_stop_active == status_word{ .state_ready_to_switch_on = 1,
                                                          .state_switched_on = 1,
                                                          .state_operation_enabled = 1,
                                                          .voltage_enabled = 1,
                                                          .state_quick_stop = 0 }
                                                 .parse_state());
static_assert(states_e::fault_reaction_active == status_word{ .state_ready_to_switch_on = 1,
                                                              .state_switched_on = 1,
                                                              .state_operation_enabled = 1,
                                                              .state_fault = 1 }
                                                     .parse_state());
static_assert(states_e::fault_reaction_active == status_word{ .state_ready_to_switch_on = 1,
                                                              .state_switched_on = 1,
                                                              .state_operation_enabled = 1,
                                                              .state_fault = 1,
                                                              .voltage_enabled = 1 }
                                                     .parse_state());
static_assert(states_e::fault_reaction_active == status_word{ .state_ready_to_switch_on = 1,
                                                              .state_switched_on = 1,
                                                              .state_operation_enabled = 1,
                                                              .state_fault = 1,
                                                              .state_quick_stop = 1 }
                                                     .parse_state());
static_assert(states_e::fault_reaction_active == status_word{ .state_ready_to_switch_on = 1,
                                                              .state_switched_on = 1,
                                                              .state_operation_enabled = 1,
                                                              .state_fault = 1,
                                                              .voltage_enabled = 1,
                                                              .state_quick_stop = 1 }
                                                     .parse_state());
static_assert(states_e::fault == status_word{ .state_fault = 1 }.parse_state());
static_assert(states_e::fault == status_word{ .state_fault = 1, .voltage_enabled = 1 }.parse_state());
static_assert(states_e::fault == status_word{ .state_fault = 1, .state_quick_stop = 1 }.parse_state());
static_assert(states_e::fault == status_word{ .state_fault = 1, .voltage_enabled = 1, .state_quick_stop = 1 }.parse_state());

}  // namespace compile_tests
