#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

#include <tfc/ec/devices/base.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/ipc_fwd.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/utils/asio_fwd.hpp>

namespace tfc::ec::devices::beckhoff {

namespace asio = boost::asio;

class el9222 final : public base {
public:
  // static constexpr size_t size = 16;
  static constexpr size_t size = 4;
  static constexpr std::string_view name{ "EL9222" };
  static constexpr auto product_code = 0x24063052;
  static constexpr uint32_t vendor_id = 0x2;

  el9222(asio::io_context& ctx, tfc::ipc_ruler::ipc_manager_client& client, uint16_t slave_index) : base(slave_index) {}

  auto setup() -> int final {
    // 1C12:0 -> 0x02
    auto work_counter = sdo_write({ 0x1C12, 0 }, static_cast<uint8_t>(0x2));
    std::cout << "work_counter: " << work_counter << std::endl;

    //   // output 1
    //   //  1C12:01 -> 0x1600
    //   work_counter = sdo_write({ 0x1C12, 1 }, static_cast<uint16_t>(0x1600));
    //   std::cout << "work_counter: " << work_counter << std::endl;

    //   // output 2
    //   // 1C12:02 -> 0x1601
    //   work_counter = sdo_write({ 0x1C12, 2 }, static_cast<uint16_t>(0x1601));
    //   std::cout << "work_counter: " << work_counter << std::endl;

    // ------------------------------------------------------------------------

    // these have errors

    // 1C13:0 TxPDO assign PDO Assign Inputs UINT8 RO 0x02 (2dec)
    work_counter = sdo_write({ 0x1C13, 0 }, static_cast<uint8_t>(0x2));
    std::cout << "work_counter: " << work_counter << std::endl;

    // input 1
    // 1C13:01 -> 0x1A00
    //  work_counter = sdo_write({ 0x1C13, 1 }, static_cast<uint16_t>(0x1a00));
    //  std::cout << "work_counter: " << work_counter << std::endl;

    //  // input 2
    //  // 1C13:02 -> 0x1A01
    //  work_counter = sdo_write({ 0x1C13, 2 }, static_cast<uint16_t>(0x1a01));
    //  std::cout << "work_counter: " << work_counter << std::endl;

    return 1;
  }

  void process_data(std::span<std::byte> input, std::span<std::byte> output) noexcept override {
    // bool button_on = input[0] && 0x01;

    // bool button_on = input[0] & 0x01;
    first_output = static_cast<uint8_t>(input[0]) & 0x01;

    std::cout << "button on : " << (first_output ? "true" : "false") << std::endl;

    std::cout << "input : " << static_cast<int>(input[0]) << std::endl;
    std::cout << "input : " << static_cast<int>(input[1]) << std::endl;

    output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0xff);

    std::cout << "output : " << static_cast<int>(output[0]) << std::endl;
    std::cout << "output : " << static_cast<int>(output[1]) << std::endl;

    std::cout << "flip: " << flip << std::endl;

    if (flip > 2000) {
      // set output to 1
      output[0] = static_cast<std::byte>(0x01);
      output[1] = static_cast<std::byte>(0x01);
      flip = 0;
    } else {
      // set output to 0
      output[0] = static_cast<std::byte>(0x00);
      output[1] = static_cast<std::byte>(0x00);
    }

    flip++;

    //  std::cout << "------------------------------------------------------------" << std::endl;
    //  std::cout << "output : " << static_cast<int>(output[0]) << std::endl;
    //  std::cout << "output : " << static_cast<int>(output[1]) << std::endl;
    //  std::cout << "input : " << static_cast<int>(input[0]) << std::endl;
    //  std::cout << "input : " << static_cast<int>(input[1]) << std::endl;
    //  std::cout << "output_states_ : " << static_cast<int>(output_states_[0]) << std::endl;
    //  std::cout << "output_states_ : " << static_cast<int>(output_states_[1]) << std::endl;
    //  std::cout << "------------------------------------------------------------" << std::endl;
  }

  auto set_output(size_t position, bool value) -> void { output_states_.set(position, value); }

private:
  std::bitset<size> output_states_;
  bool first_output;
  int flip;
  int out_0;
  int out_1;
  int in_0;
  int in_1;
  int output_states_0;
  int output_states_1;
};

}  // namespace tfc::ec::devices::beckhoff
