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
  static constexpr size_t size = 2;
  static constexpr std::string_view name{ "EL9222" };
  static constexpr auto product_code = 0x24063052;
  static constexpr uint32_t vendor_id = 0x2;

  el9222(asio::io_context& ctx, tfc::ipc_ruler::ipc_manager_client& client, uint16_t slave_index) : base(slave_index) {}

  auto setup() -> int final {
    // You might need to write 0 du 0x1c13:0 first, update 0x1c13:x , fnalize with 0x1c13:0 = number of entries in 0x1c13

    // nominal current
    // n=0 for ch1, n=1 for ch2
    // 80n0:11
    sdo_write({ 0x8000, 11 }, std::uint8_t{ 0x1 });
    sdo_write({ 0x8010, 11 }, std::uint8_t{ 0x1 });

    // sdo_write(ecx::rx_pdo_assign<0x00>, uint8_t{ 0 });

    //  for (uint8_t i = 0; i < size; i++) {
    //    sdo_write({ 0x1C13, i + 1 }, static_cast<uint16_t>(0x1A00 + (i * 2) + 1));
    //  }

    //  sdo_write({ 0x1C13, 0 }, 2);

    return 1;
  }

  void process_data(std::span<std::byte>, std::span<std::byte> output) noexcept override {
    output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0xff);
    if constexpr (size > 8) {
      output[1] = static_cast<std::byte>(output_states_.to_ulong() >> 8);
    }
  }

  auto set_output(size_t position, bool value) -> void { output_states_.set(position, value); }

private:
  std::bitset<size> output_states_;
};

}  // namespace tfc::ec::devices::beckhoff
