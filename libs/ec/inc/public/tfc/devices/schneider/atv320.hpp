#pragma once

#include <fmt/format.h>
#include <bitset>
#include <memory>
#include <optional>

#include "tfc/devices/base.hpp"

#include "tfc/cia/402.hpp"
#include "tfc/ecx.hpp"

class atv320 : public device_base {
public:
  static constexpr auto vendor_id = 0x0800005a;
  static constexpr auto product_code = 0x00000389;
  auto process_data(uint8* input, uint8* output) noexcept -> void override {
    if (input == nullptr) {
      return;
    }
    // All registers in the ATV320 ar uint16, create a pointer to this memory
    // With the same size
    uint16_t* in = reinterpret_cast<uint16_t*>(input);  // NOLINT
    bool chg = false;
    if (status_word_ == in[0] || current_speed_ == in[1] || current_usage_ == in[2] || digital_inputs_ == in[3] ||
        analog_input_1_ == in[4] || analog_input_3_ == in[5]) {
      chg = true;
    }
    status_word_ = in[0];
    current_speed_ = in[1];
    current_usage_ = in[2];
    digital_inputs_ = in[3];
    analog_input_1_ = in[4];
    analog_input_3_ = in[5];

    auto reference_trajectory = static_cast<uint16_t>(500.0 * 125.0 / 0xff);
    auto state = tfc::ec::cia_402::parse_state(status_word_);
    using tfc::ec::cia_402::commands_e;
    using tfc::ec::cia_402::states_e;
    auto command = tfc::ec::cia_402::transition(state);

    if (chg) {
      printf("state %s\tcommand: %s\tcurrent_speed: %d\tana1: 0x%x\tana3: 0x%x\tdigital: %s\n",
             tfc::ec::cia_402::to_string(state).c_str(), tfc::ec::cia_402::to_string(command).c_str(),
             current_speed_, analog_input_1_, analog_input_3_, std::bitset<6>(digital_inputs_).to_string().c_str());
    }
    auto* data_ptr = reinterpret_cast<uint16_t*>(output);
    *data_ptr++ = static_cast<uint16_t>(command);
    *data_ptr++ = reference_trajectory;
  }
  auto setup(ecx_contextt* context, uint16_t slave) -> int override {
    // Set PDO variables
    // Clean rx and tx prod assign
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_assign<0x00>, 0);
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_assign<0x00>, 0);

    // Zero the size
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_mapping<0x00>, 0);
    // Assign rx variables
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x01>, 0x60410010);  // ETA  - STATUS WORD
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x02>, 0x20020310);  // RFR  - CURRENT SPEED HZ
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x03>, 0x20020510);  // LCR  - CURRENT USAGE ( A )
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x04>, 0x20160310);  // 1LIR - DI1-DI6
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x05>, 0x20162B10);  // AI1C - Physical value AI1
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x06>, 0x20162D10);  // AI3C - Physical value AI3
    // Set rx size
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_mapping<0x00>, 5);

    // Zero the size
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_mapping<0x00>, 0);
    // Assign tx variables
    ecx::sdo_write<uint32_t>(context, slave, ecx::tx_pdo_mapping<0x01>, 0x60400010);  // CMD - CONTROL WORD
    ecx::sdo_write<uint32_t>(context, slave, ecx::tx_pdo_mapping<0x02>, 0x20370310);  // LFR - REFERENCE SPEED HZ
    ecx::sdo_write<uint32_t>(
        context, slave, ecx::tx_pdo_mapping<0x03>,
        0x20160D10);  // OL1R - Logic outputs states ( bit0: Relay 1, bit1: Relay 2, bit3 - bit7: unknown, bit8: DQ1 )
    ecx::sdo_write<uint32_t>(context, slave, ecx::tx_pdo_mapping<0x03>, 0x20164810);  // AO1C - AQ1 physical value

    // Set tx size
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_mapping<0x00>, 3);

    // Assign pdo's to mappings
    ecx::sdo_write<uint16_t>(context, slave, ecx::rx_pdo_assign<0x01>, ecx::rx_pdo_mapping<>.first);
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_assign<0x00>, 1);

    ecx::sdo_write<uint16_t>(context, slave, ecx::tx_pdo_assign<0x01>, ecx::tx_pdo_mapping<>.first);
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_assign<0x00>, 1);

    // Clear internal ATV Functionality for outputs and inputs
    // Disconnect relay 1 from fault assignment
    ecx::sdo_write<uint16_t>(context, slave, { 0x2014, 0x02 }, 0);  // 0 - Not configured
    // Disconnect analog output 1 from frequency
    ecx::sdo_write<uint16_t>(context, slave, { 0x2014, 0x16 }, 0);  // 0 - Not configured
    // Set AO1 output to current
    ecx::sdo_write<uint16_t>(context, slave, { 0x2010, 0x02 }, 2);  // 2 - Current
    // Set AI1 input to current
    ecx::sdo_write<uint16_t>(context, slave, { 0x200E, 0x03 }, 2);  // 2 - Current

    // test writing alias address - this does not seem to work. Direct eeprom writing also possible working.
    // ecx::sdo_write<uint16_t>(context, slave, { 0x2024, 0x92 }, 1337);  // 2 - Current

    // asign motor parameters from config.
    return 1;
  }

private:
  uint16_t status_word_;
  uint16_t current_speed_;
  uint16_t current_usage_;
  uint16_t analog_input_1_;
  uint16_t analog_input_3_;
  uint16_t digital_inputs_;
};
