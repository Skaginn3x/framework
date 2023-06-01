#pragma once

#include <cstdint>
#include <string>

namespace tfc::ec::cia_402 {

enum struct commands_e : uint16_t {
  disable_voltage = 0x0000,
  shutdown = 0x0006,
  quick_stop = 0x0002,
  switch_on = 0x0007,
  enable_operation = 0x000f,
  fault_reset = 0x0080
};

[[maybe_unused]] static auto to_string(commands_e value) -> std::string {
  using std::string_literals::operator""s;
  switch (value) {
    case commands_e::disable_voltage:
      return "Disable voltage"s;
    case commands_e::shutdown:
      return "Shutdown"s;
    case commands_e::quick_stop:
      return "Quick stop"s;
    case commands_e::switch_on:
      return "Switch on"s;
    case commands_e::enable_operation:
      return "Enable operation"s;
    case commands_e::fault_reset:
      return "Fault reset"s;
  }
  return "Unknown"s;
}

enum struct states_e : uint16_t {
  not_ready_to_switch_on = 1,
  switch_on_disabled = 2,
  ready_to_switch_on = 3,
  switched_on = 4,
  operation_enabled = 5,
  quick_stop_active = 6,
  fault_reaction_active = 7,
  fault = 8
};

struct status_word {
  uint8_t state_ready_to_switch_on : 1 {};
  uint8_t state_switched_on : 1 {};
  uint8_t state_operation_enabled : 1 {};
  uint8_t state_fault : 1 {};
  uint8_t voltage_enabled : 1 {};
  uint8_t state_quick_stop : 1 {};
  uint8_t state_switch_on_disabled : 1 {};
  uint8_t warning : 1 {};
  uint8_t halt_request_active : 1 {};
  uint8_t remote : 1 {};
  uint8_t target_reached : 1 {};
  uint8_t internal_limit_active : 1 {};
  uint8_t application_specific_0 : 1 {}; // 12th bit
  uint8_t application_specific_1 : 1 {};
  uint8_t application_specific_2 : 1 {};
  uint8_t application_specific_3 : 1 {};
  constexpr explicit operator uint16_t() const noexcept {
    std::bitset<16> output{};
    output.set(0, state_ready_to_switch_on);
    output.set(1, state_switched_on);
    output.set(2, state_operation_enabled);
    output.set(3, state_fault);
    output.set(4, voltage_enabled);
    output.set(5, state_quick_stop);
    output.set(6, state_switch_on_disabled);
    output.set(7, warning);
    output.set(8, halt_request_active);
    output.set(9, remote);
    output.set(10, target_reached);
    output.set(11, internal_limit_active);
    output.set(12, application_specific_0);
    output.set(13, application_specific_1);
    output.set(14, application_specific_2);
    output.set(15, application_specific_3);
    return static_cast<uint16_t>(output.to_ulong());
  }
  constexpr explicit operator states_e() const noexcept {
    // clang-format off
    if (state_ready_to_switch_on) return states_e::ready_to_switch_on;
    if (state_switched_on) return states_e::switched_on;
    if (state_operation_enabled) return states_e::operation_enabled;
    if (state_fault) return states_e::fault;
    if (state_quick_stop) return states_e::quick_stop_active;
    // clang-format on
    return states_e::not_ready_to_switch_on;
  }
};

static_assert(sizeof(status_word) == 2);
static_assert(static_cast<states_e>(status_word{.state_ready_to_switch_on=1}) == states_e::ready_to_switch_on);
static_assert(static_cast<states_e>(status_word{.state_switched_on=1}) == states_e::switched_on);
static_assert(static_cast<states_e>(status_word{.state_operation_enabled=1}) == states_e::operation_enabled);
static_assert(static_cast<states_e>(status_word{.state_fault=1}) == states_e::fault);
static_assert(static_cast<states_e>(status_word{.state_quick_stop=1}) == states_e::quick_stop_active);


[[maybe_unused]] static auto to_string(states_e value) -> std::string {
  using std::string_literals::operator""s;
  switch (value) {
    case states_e::not_ready_to_switch_on:
      return "Not ready to switch on"s;
    case states_e::switch_on_disabled:
      return "Switch on disabled"s;
    case states_e::ready_to_switch_on:
      return "Ready to switch on"s;
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
inline auto parse_state(uint16_t const& status_word) -> states_e {
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
inline constexpr auto parse_state(status_word status) -> states_e {
  if (status.state_ready_to_switch_on) {
    return states_e::ready_to_switch_on;
  }
  return states_e::not_ready_to_switch_on; //
//  return parse_state(static_cast<uint16_t>(status)); // todo maybe erase the function above and implement here
}


/**
 * Transition to operational mode
 * @param current_state State parsed from status word determined to be the current status of a drive
 * @param quick_stop if the drive should be placed in quick_stop mode.
 * @return the command to transition to operational mode / stick in quick stop mode.
 */
inline auto transition(states_e current_state, bool quick_stop) -> commands_e {
  switch (current_state) {
    case states_e::switch_on_disabled:
      return commands_e::shutdown;
    case states_e::ready_to_switch_on:
      return commands_e::enable_operation;  // This is a shortcut marked as 3B in ethercat manual for atv320
    case states_e::operation_enabled:
      if (quick_stop) {
        return commands_e::quick_stop;
      }
      return commands_e::enable_operation;
    case states_e::switched_on:
      return commands_e::enable_operation;
    case states_e::fault:
      return commands_e::fault_reset;
    case states_e::quick_stop_active:
      if (quick_stop) {
        return commands_e::quick_stop;
      }
      return commands_e::disable_voltage;
    case states_e::not_ready_to_switch_on:
    case states_e::fault_reaction_active:
      return commands_e::disable_voltage;
  }
  // Can only occur if someone casts an integer for state_e that is not defined in the enum
  return commands_e::disable_voltage;
}

}  // namespace tfc::ec::cia_402
