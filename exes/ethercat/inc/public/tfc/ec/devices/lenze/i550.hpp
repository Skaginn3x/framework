#pragma once

#include <chrono>
#include "tfc/ec/devices/base.hpp"
#include "i550/pdo.hpp"

namespace tfc::ec::devices::lenze::i550 {
template <uint8_t subindex = 0>
static constexpr ecx::index_t rx_pdo_mapping_i550 = { 0x1605, subindex };  // todo @omar where did you find rx pdo as 1a00
/// tx for slave, rx for master
template <uint8_t subindex = 0>
static constexpr ecx::index_t tx_pdo_mapping_i550 = { 0x1A05, subindex };
class i550 final : public base {
public:
  ~i550() final {}
  explicit i550(uint16_t slave_index) : base(slave_index) {}
  void process_data(std::span<std::byte> input, std::span<std::byte> output) final {
    // if (input.size() != sizeof(input_t)) {
    //   logger_.error("Input data size mismatch: expected {}, got {}", sizeof(input_t), input.size());
    //   return;
    // }
    if (input.size() != 6 || output.size() != 4) {
      logger_.error("OMG WRONG SIZES input size {}, output size {}", input.size(), output.size());
      return;
    }

    const input_t* in = std::launder(reinterpret_cast<input_t*>(input.data()));
    output_t* out = std::launder(reinterpret_cast<output_t*>(output.data()));

    out->rpm = 1500;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    if (now % 10 < 5) {
      out->control = cia_402::transition(in->status_word.parse_state(), cia_402::transition_action::run, true);
    } else {
      out->control = cia_402::transition(in->status_word.parse_state(), cia_402::transition_action::stop, true);
    }

  }

  auto setup() -> int final {
    // Set PDO variables
    // Clean rx and tx prod assign
    sdo_write<uint8_t>(ecx::rx_pdo_assign<0x00>, 0);
    sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 0);
    // // // Zero the size
    sdo_write<uint8_t>( rx_pdo_mapping_i550<0x00>, 0);
    sdo_write<uint32_t>(rx_pdo_mapping_i550<0x01>, 0x60400010);  // CMD - Control Word
    sdo_write<uint32_t>(rx_pdo_mapping_i550<0x02>, 0x60420010);  // set speed
    // // // sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x03>, 0x20020510);  // LCR  - CURRENT USAGE ( A
    // // // sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x04>, 0x20160310);  // 1LIR - DI1-DI6
    // // // sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x05>, 0x20291610);  // LFT  - Last error occured
    // // // sdo_write<uint32_t>(ecx::tx_pdo_mapping<0x06>, 0x20022910);  // HMIS - Drive state
    sdo_write<uint8_t>(rx_pdo_mapping_i550<0x00>, 2);

    // // // Zero the size
    sdo_write<uint8_t>(tx_pdo_mapping_i550<0x00>, 0);
    // // // Assign tx variables
    sdo_write<uint32_t>(tx_pdo_mapping_i550<0x01>, 0x60410010);  // ETA  - STATUS WORD
    sdo_write<uint32_t>(tx_pdo_mapping_i550<0x02>, 0x60440010);  // Actual speed
    sdo_write<uint32_t>(tx_pdo_mapping_i550<0x03>, 0x603F0010);  // Error
    // // sdo_write<uint32_t>(
    // //     ecx::rx_pdo_mapping<0x03>,
    // //     0x20160D10);  // OL1R - Logic outputs states ( bit0: Relay 1, bit1: Relay 2, bit3 - bit7: unknown, bit8: DQ1 )
    // // sdo_write<uint32_t>(ecx::rx_pdo_mapping<0x04>, 0x203C0210);  // ACC - Acceleration
    // // sdo_write<uint32_t>(ecx::rx_pdo_mapping<0x05>, 0x203C0310);  // DEC - Deceleration

    // // Set tx size
    sdo_write<uint8_t>(tx_pdo_mapping_i550<0x00>, 3);

    // Assign pdo's to mappings
    sdo_write<uint16_t>(ecx::rx_pdo_assign<0x01>, rx_pdo_mapping_i550<>.first);
    sdo_write<uint8_t>(ecx::rx_pdo_assign<0x00>, 1);

    sdo_write<uint16_t>(ecx::tx_pdo_assign<0x01>, tx_pdo_mapping_i550<>.first);
    sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 1);

    // sdo_write<uint8_t>(ecx::index_t(0x6060, 0x00), 2); // Set operating mode to cia402 velocity mode
    sdo_write<uint8_t>(ecx::index_t(0x2631, 0x01), 1); // Set enable inverter to true
    sdo_write<uint8_t>(ecx::index_t(0x2631, 0x02), 1); // Set allow run to constant true
    return 1;
  }
  static constexpr uint32_t product_code = 0x69055000;
  static constexpr uint32_t vendor_id = 0x3b;
};
}  // namespace tfc::ec::devices::lenze
