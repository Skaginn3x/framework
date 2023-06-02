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
  uint8_t application_specific_0 : 1 {};  // 12th bit
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
    /*
      Status Word Bit Mapping:
      Bit 0: Ready to switch on.
      Bit 1: Switched on.
      Bit 2: Operation enabled.
      Bit 3: Fault.
      Bit 4: Voltage enabled.
      Bit 5: Quick stop. (FYI, enabled low)
      Bit 6: Switch on disabled.

      Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 | State
      ------------------------------------------------------------
        0   |   x   |   x   |   0   |   0   |   0   |   0   | State 1: Not ready to switch on
        1   |   x   |   x   |   0   |   0   |   0   |   0   | State 2: Switch on disabled
        0   |   1   |   x   |   0   |   0   |   0   |   1   | State 3: Ready to switch on
        0   |   1   |   1   |   0   |   0   |   1   |   1   | State 4: Switched on
        0   |   1   |   1   |   0   |   1   |   1   |   1   | State 5: Operation enabled
        0   |   0   |   1   |   0   |   1   |   1   |   1   | State 6: Quick stop active
        0   |   x   |   x   |   1   |   1   |   1   |   1   | State 7: Fault reaction active
        0   |   x   |   x   |   1   |   0   |   0   |   0   | State 8: Fault
    */
    if (!state_fault) {
      if (!state_ready_to_switch_on && !state_switched_on && !state_operation_enabled && !state_switch_on_disabled) {
        return states_e::not_ready_to_switch_on;
      }
      if (state_switch_on_disabled) {
        return states_e::switch_on_disabled;
      }
      if (state_quick_stop) {
        if (state_ready_to_switch_on && state_quick_stop && !state_switched_on) {
          return states_e::ready_to_switch_on;
        }
        if (state_ready_to_switch_on && state_switched_on && voltage_enabled) {
          if (state_operation_enabled) {
            return states_e::operation_enabled;
          }
          return states_e::switched_on;
        }
      } else {
        return states_e::quick_stop_active;
      }
    } else {
      if (state_operation_enabled && state_switched_on && state_ready_to_switch_on) {
        return states_e::fault_reaction_active;
      }
      return states_e::fault;
    }
    return states_e::not_ready_to_switch_on;
  }
  static constexpr auto from_uint(uint16_t word) -> status_word {
    std::bitset<16> word_bitset{ word };
    status_word out{};
    out.state_ready_to_switch_on = word_bitset[0];
    out.state_switched_on = word_bitset[1];
    out.state_operation_enabled = word_bitset[2];
    out.state_fault = word_bitset[3];
    out.voltage_enabled = word_bitset[4];
    out.state_quick_stop = word_bitset[5];
    out.state_switch_on_disabled = word_bitset[6];
    out.warning = word_bitset[7];
    out.halt_request_active = word_bitset[8];
    out.remote = word_bitset[9];
    out.target_reached = word_bitset[10];
    out.internal_limit_active = word_bitset[11];
    out.application_specific_0 = word_bitset[12];
    out.application_specific_1 = word_bitset[13];
    out.application_specific_2 = word_bitset[14];
    out.application_specific_3 = word_bitset[15];
    return out;
  }
};

static_assert(sizeof(status_word) == 2);

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
 * @param word ETA of the vfd/servo
 * @return cia 402 state drive status_word represents
 */
inline auto parse_state(status_word word) -> states_e {
  return static_cast<states_e>(word);
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
