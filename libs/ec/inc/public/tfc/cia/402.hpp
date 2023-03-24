#pragma once

#include <boost/sml.hpp>
#include <cstdint>

namespace tfc::ec::cia_402 {

enum commands_e : uint16_t {
  disable_voltage = 0x0000,
  shutdown = 0x0006,
  quick_stop = 0x0002,
  switch_on = 0x0007,
  enable_operation = 0x000f,
  fault_reset = 0x0080
};

auto to_string(commands_e value) -> std::string {
  using std::string_literals::operator""s;
  switch (value) {
    case disable_voltage:
      return "Disable voltage"s;
    case shutdown:
      return "Shutdown"s;
    case quick_stop:
      return "Quick stop"s;
    case switch_on:
      return "Switch on"s;
    case enable_operation:
      return "Enable operation"s;
    case fault_reset:
      return "Fault reset"s;
  }
  return "Unknown"s;
}

enum struct states_e {
  not_ready_to_switch_on = 1,
  switch_on_disabled = 2,
  ready_to_switch_on = 3,
  switched_on = 4,
  operation_enabled = 5,
  quick_stop_active = 6,
  fault_reaction_active = 7,
  fault = 8
};

auto to_string(states_e value) -> std::string {
  using std::string_literals::operator""s;
  switch (value) {
    case states_e::not_ready_to_switch_on:
      return "Not ready to switch on"s;
    case states_e::switch_on_disabled:
      return "Switch on disabled"s;
    case states_e::ready_to_switch_on:
      return "ready to switch on"s;
    case states_e::switched_on:
      return "Switched on"s;
    case states_e::operation_enabled:
      return "Operation enabled"s;
    case states_e::quick_stop_active:
      return "Quick stop active"s;
    case states_e::fault_reaction_active:
      return "Fault reaction active"s;
    case states_e::fault:
      return "Fault"s;
  }
  return "unknown"s;
}

/**
 * @param status_word ETA of the vfd/servo
 * @return cia 402 state drive status_word represents
 */
auto parse_state(uint16_t const& status_word) -> states_e {
  uint16_t const status_low = status_word & 0xff;
  switch (status_low % 0x10) {
    case 0x01:
      return states_e::ready_to_switch_on;
    case 0x00:
      return states_e::switch_on_disabled;
    case 0x08:
      return states_e::fault;
    case 0x03:
      return states_e::switched_on;
    case 0x07:
      switch (status_low / 0x10) {
        case 0x03:
          return states_e::operation_enabled;
        case 0x01:
          return states_e::quick_stop_active;
        default:
          return states_e::not_ready_to_switch_on;
      }
    default:
      return states_e::not_ready_to_switch_on;
  }
}

/**
 * Provide the command for transitioning to the destination state.
 * For destination_state = Fault reaction active, Fault, Not ready to switch on.
 * The function will return the command for transitioning to switch on disabled ( Disable voltage ).
 * @param current_state
 * @param destination_state
 * @return Returns the command to apply in trying to move the vfd/servo closer to the destination state
 */
auto transition(states_e current_state, states_e destination_state) -> commands_e {
  if (current_state == destination_state) {
    return commands_e::shutdown;
  }
  return commands_e::shutdown;
}
/**
 * Transition to operational mode
 * @param current_state
 * @return the command to transition to operational mode
 */
auto transition(states_e current_state) -> commands_e {
  switch (current_state) {
    case states_e::switch_on_disabled:
      return commands_e::shutdown;
    case states_e::ready_to_switch_on:
      return commands_e::switch_on;
    case states_e::operation_enabled:
    case states_e::switched_on:
      return commands_e::enable_operation;
    case states_e::fault:
      return commands_e::fault_reset;
    default:
      return static_cast<commands_e>(0);
  }
}

}  // namespace tfc::ec::cia_402