#pragma once

#include <string>

#include "tfc/ec/devices/util.hpp"
#include "tfc/ec/soem_interface.hpp"

namespace tfc::ec::devices::beckhoff {
struct siemens_status {
  bool out_of_range = false;
  bool unknown_1 = false;
  bool unknown_2 = false;
};

template <uint8_t size, auto p_code>
class el305x : public base {
public:
  explicit el305x(boost::asio::io_context&, uint16_t const slave_index) : base(slave_index) {}
  static constexpr uint32_t product_code = p_code;
  static constexpr uint32_t vendor_id = 0x2;

  auto setup() -> int final {
    // Clean rx pdo assign
    sdo_write(ecx::rx_pdo_assign<0x00>, uint8_t{ 0 });
    // Set siemens bits true
    // and enable compact mode
    // Each input settings field is in 0x8000 + offset * 0x10
    // So 0 -> 0x80000
    // 1 -> 0x8010
    // 2 -> 0x8020
    // This depends on size
    for (uint8_t i = 0; i < size; i++) {
      // Set rx pdo to compact mode
      sdo_write({ 0x1C13, i + 1 }, static_cast<uint16_t>(0x1A00 + (i * 2) + 1));

      uint16_t const settings_index = 0x8000 + (static_cast<uint16_t>(i) * 0x10);
      sdo_write({ settings_index, 0x05 }, static_cast<uint8_t>(true));  // Enable - siemens mode
    }
    // Set rx pdo size to size
    sdo_write(ecx::rx_pdo_assign<0x00>, size);

    // printf("Processdatasize: %d", context->slavelist[slave].Obytes);
    // This is can be used to restore default parameters
    // ecx::sdo_write<uint32_t>(context, slave, { 0x1011, 0x01 }, 0x64616F6C );  // RESTORE PARAMETERS TO DELIVERY STATE
    return 1;
  }

  void process_data(std::span<std::byte> input, std::span<std::byte>) noexcept final {
    // Cast pointer type to uint16_t
    std::span<uint16_t> input_aligned(reinterpret_cast<uint16_t*>(input.data()), input.size() / 2);

    // The value is setup in compact mode and siemens bits
    // are enabled. This means that there are three
    // status bits in the LSB the other 13 are split
    // up as one unused sign bit and then 12 bits data.
    for (size_t i = 0; i < size; i++) {
      uint16_t const raw_value = input_aligned[i];
      std::bitset<3> const status_bits(raw_value);
      status_[i] = siemens_status{ .out_of_range = status_bits.test(0),
                                   .unknown_1 = status_bits.test(1),
                                   .unknown_2 = status_bits.test(2) };
      // value_[i] = units::isq::si::electric_current<units::isq::si::milliampere>(map<double>(raw_value >> 3, 0, 4096, 4,
      // 20));
    }
    // size_t counter = 0;
    // for (auto milli_ampere : value_) {
    //   auto error_bits = std::bitset<3>(milli_ampere);
    //   value_[counter] = milli_ampere >> 3;
    //   double m_amp = map<uint64_t>(value_[counter], 0, 4096, 4000, 20000);
    //   printf("(%lu)->0x%x\t0x%x\t%s\t%f°\t", counter, milli_ampere, value_[counter], error_bits.to_string().c_str(),
    //          celsius);
    //   counter++;
    // }
  }

private:
  std::array<siemens_status, size> status_;  // Three status bits
  // std::array<units::isq::si::electric_current<units::isq::si::milliampere>, size> value_;
};

using el3054 = el305x<4, 0xbee3052>;

}  // namespace tfc::ec::devices::beckhoff
