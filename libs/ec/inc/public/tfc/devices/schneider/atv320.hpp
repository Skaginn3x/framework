#pragma once

#include <fmt/format.h>
#include <bitset>
#include <memory>
#include <optional>

#include "tfc/devices/base.hpp"

#include "tfc/cia/402.hpp"
#include "tfc/ecx.hpp"
#include "tfc/ipc.hpp"

namespace tfc::ec::devices::schneider {
class atv320 : public base {
public:
  explicit atv320(boost::asio::io_context& ctx) : base(ctx) {
    state_transmitter_ = tfc::ipc::string_send::create(ctx_, "atv320.string.state").value();
    command_transmitter_ = tfc::ipc::string_send::create(ctx_, "atv320.string.command").value();
    for (size_t i = 0; i < 6; i++) {
      di_transmitters_.emplace_back(tfc::ipc::bool_send::create(ctx_, fmt::format("atv320.bool.in.{}", i)).value());
    }
    for (size_t i = 0; i < 2; i++) {
      ai_transmitters_.emplace_back(tfc::ipc::int_send::create(ctx_, fmt::format("atv320.int.in.{}", i)).value());
    }
    quick_stop_recv_ = tfc::ipc::bool_recv_cb::create(ctx_, "atv320.bool.quick_stop");
    quick_stop_recv_->init(fmt::format("{}.{}.easyecat.1.bool.in.0", tfc::base::get_exe_name(), tfc::base::get_proc_name()),
                           [this](bool value) { quick_stop_ = value; });
    reference_frequency_recv_ = tfc::ipc::double_recv_cb::create(ctx_, "atv320.double.out.freq");
    reference_frequency_recv_->init("tfcctl.def.atv320.freq.double",
                                    [this](double value) { reference_frequency_ = static_cast<int16_t>(value * 10.0); });
  }
  static constexpr auto vendor_id = 0x0800005a;
  static constexpr auto product_code = 0x00000389;
  auto process_data(uint8* input, uint8* output) noexcept -> void override {
    if (input == nullptr || output == nullptr) {
      std::cerr << "ATV320 in/out, nullptr" << std::endl;
      return;
    }
    // All registers in the ATV320 ar uint16, create a pointer to this memory
    // With the same size
    uint16_t* in = reinterpret_cast<uint16_t*>(input);  // NOLINT
    status_word_ = in[0];

    auto state = tfc::ec::cia_402::parse_state(status_word_);
    if (cia_402::to_string(state) != last_state_) {
      state_transmitter_->async_send(cia_402::to_string(state), async_send_callback);
    }
    std::bitset<6> const value(in[3]);
    for (size_t i = 0; i < 6; i++) {
      if (value.test(i) != last_bool_values_.test(i)) {
        di_transmitters_[i]->async_send(value.test(i), async_send_callback);
      }
    }
    last_bool_values_ = value;

    auto* analog_ptr = reinterpret_cast<int16_t*>(in) + 4;
    for (size_t i = 0; i < 2; i++) {
      if (last_analog_inputs_[i] != analog_ptr[i]) {
        ai_transmitters_[i]->async_send(analog_ptr[i], async_send_callback);
      }
      last_analog_inputs_[i] = analog_ptr[i];
    }

    using tfc::ec::cia_402::commands_e;
    using tfc::ec::cia_402::states_e;
    auto command = tfc::ec::cia_402::transition(state, quick_stop_);

    if (cia_402::to_string(command) != last_command_) {
      command_transmitter_->async_send(cia_402::to_string(command), async_send_callback);
    }

    auto* data_ptr = reinterpret_cast<uint16_t*>(output);
    *data_ptr++ = static_cast<uint16_t>(command);
    *data_ptr++ = quick_stop_ ? 0 : reference_frequency_;
  }
  static auto async_send_callback(std::error_code const& error, size_t) -> void {
    if (error) {
      printf("ATV320, error transmitting : %s\n", error.message().c_str());
    }
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
    // ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x06>, 0x20165E10);  // AI3C - Physical value without
    // filter AI3
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x06>, 0x20162D10);  // AI3C - Physical value AI3
    // ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x06>, 0x20162310);  // AI3C - Standardized value AI3
    //  Set rx size
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_mapping<0x00>, 6);

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
    // Set AI3 input to positive
    ecx::sdo_write<uint16_t>(context, slave, { 0x200E, 0x55 }, 0);

    // test writing alias address - this does not seem to work. Direct eeprom writing also possible working.
    // ecx::sdo_write<uint16_t>(context, slave, { 0x2024, 0x92 }, 1337);  // 2 - Current

    // assign motor parameters from config.
    return 1;
  }

private:
  uint16_t status_word_;
  std::array<int16_t, 2> last_analog_inputs_;
  std::vector<std::shared_ptr<tfc::ipc::int_send>> ai_transmitters_;
  std::bitset<6> last_bool_values_;
  std::vector<std::shared_ptr<tfc::ipc::bool_send>> di_transmitters_;
  std::string last_state_;
  std::shared_ptr<tfc::ipc::string_send> state_transmitter_;
  std::string last_command_;
  std::shared_ptr<tfc::ipc::string_send> command_transmitter_;
  bool quick_stop_ = false;
  std::shared_ptr<tfc::ipc::bool_recv_cb> quick_stop_recv_;
  int16_t reference_frequency_;
  std::shared_ptr<tfc::ipc::double_recv_cb> reference_frequency_recv_;
};
}  // namespace tfc::ec::devices::schneider