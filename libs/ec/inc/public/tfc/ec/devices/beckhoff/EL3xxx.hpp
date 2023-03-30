#pragma once

#include <units/isq/si/electric_current.h>
#include <units/quantity_io.h>
#include <string>

#include "tfc/ec/devices/util.hpp"
#include "tfc/ec/soem_interface.hpp"

namespace tfc::ec::devices::beckhoff {
struct siemens_status {
  bool out_of_range = false;
  bool unknown_1 = false;
  bool unknown_2 = false;
};

using units::isq::si::electric_current_references::mA;

template <size_t size, auto p_code>
class el305x : public base {
public:
  explicit el305x(boost::asio::io_context&, uint16_t const slave_index) : base(slave_index) {}
  static constexpr uint32_t product_code = p_code;
  static constexpr uint32_t vendor_id = 0x2;

  auto setup(ecx_contextt* context, uint16_t slave) -> int final {
    // Clean rx pdo assign
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_assign<0x00>, 0);
    // Set siemens bits true
    // and enable compact mode
    // Each input settings field is in 0x8000 + offset * 0x10
    // So 0 -> 0x80000
    // 1 -> 0x8010
    // 2 -> 0x8020
    // This depends on size
    for (size_t i = 0; i < size; i++) {
      // Set rx pdo to compact mode
      ecx::sdo_write<uint16_t>(context, slave, { 0x1C13, i + 1 }, 0x1A00 + (i * 2) + 1);

      uint16_t const settings_index = 0x8000 + (i * 0x10);
      auto wkc = ecx::sdo_write<bool>(context, slave, { settings_index, 0x05 }, true);  // Enable - siemens mode
    }
    // Set rx pdo size to size
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_assign<0x00>, size);

    // printf("Processdatasize: %d", context->slavelist[slave].Obytes);
    // This is can be used to restore default parameters
    // ecx::sdo_write<uint32_t>(context, slave, { 0x1011, 0x01 }, 0x64616F6C );  // RESTORE PARAMETERS TO DELIVERY STATE
    return 1;
  }

  void process_data(uint8_t* input, uint8_t*) noexcept final {
    // Cast pointer type to uint16_t
    auto* in_ptr = reinterpret_cast<int16_t*>(input);

    // The value is setup in compact mode and siemens bits
    // are enabled. This means that there are three
    // status bits in the LSB the other 13 are split
    // up as one unused sign bit and then 12 bits data.
    for (size_t i = 0; i < size; i++) {
      uint16_t const raw_value = *in_ptr++;
      std::bitset<3> const status_bits(raw_value);
      status_[i] = siemens_status{ .out_of_range = status_bits.test(0),
                                   .unknown_1 = status_bits.test(1),
                                   .unknown_2 = status_bits.test(2) };
      value_[i] = units::isq::si::electric_current<units::isq::si::milliampere>(map<double>(raw_value >> 3, 0, 4096, 4, 20));
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
  std::array<units::isq::si::electric_current<units::isq::si::milliampere>, size> value_;
};

using el3054 = el305x<4, 0xbee3052>;

}  // namespace tfc::ec::devices::beckhoff