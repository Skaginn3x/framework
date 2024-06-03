#pragma once

#include <cstdint>
#include <string>

#include <mp-units/systems/si/si.h>
#include <mp-units/systems/si/units.h>
#include <glaze/util/string_literal.hpp>

#include "tfc/ec/devices/util.hpp"
#include "tfc/ec/soem_interface.hpp"
#include "tfc/stx/to_string_view.hpp"

namespace tfc::ec::devices::beckhoff {
struct siemens_status {
  bool out_of_range = false;
  bool unknown_1 = false;
  bool unknown_2 = false;
};

// Cast pointer type to uint16_t
// The value is setup in compact mode and siemens bits
// are enabled. This means that there are three
// status bits in the LSB the other 13 are split
// up as one unused sign bit and then 12 bits data.
#pragma pack(push, 1)
struct compact_value {
  bool out_of_range : 1;
  bool unknown_1 : 1;
  bool unknown_2 : 1;
  std::uint16_t value : 13;
};

// TODO: Remove this when compact mode is enabled.
struct temporary {
  std::uint16_t unused;
  compact_value value;
};
#pragma pack(pop)
static_assert(sizeof(compact_value) == 2);
static_assert(sizeof(temporary) == 4);

template <uint8_t size, auto p_code>
class el305x final : public base<el305x<size, p_code>> {
public:
  explicit el305x(boost::asio::io_context&, uint16_t const slave_index) : base<el305x>(slave_index) {}
  static constexpr uint32_t product_code = p_code;
  static constexpr uint32_t vendor_id = 0x2;

  static constexpr std::string_view name{ glz::join_v<glz::chars<"EL305">, stx::to_string_view_v<size>> };

  auto setup_driver() -> int {
    // set tx pdo assign to 0
    // this->template sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, uint8_t{ 0 });
    // Set siemens bits true
    // and enable compact mode
    // Each input settings field is in 0x8000 + offset * 0x10
    // So 0 -> 0x80000
    // 1 -> 0x8010
    // 2 -> 0x8020
    // This depends on size
    for (uint8_t i = 0; i < size; i++) {
      // Set rx pdo to compact mode
      // this->sdo_write({0x1C13, i+1}, uint32_t{ 0x1A00U + (i * 2U) + 1U });

      uint16_t const settings_index = 0x8000 + (static_cast<uint16_t>(i) * 0x10);
      this->template sdo_write<uint8_t>({ settings_index, 0x05 }, static_cast<uint8_t>(true));  // Enable - siemens mode
    }
    // Set rx pdo size to size

    // Assign the compact PDO mapping
    // this->template sdo_write<uint16_t>(ecx::tx_pdo_assign<0x01>, 0x1A01);
    // this->template sdo_write<uint8_t>(ecx::tx_pdo_assign<0x00>, 1);

    // printf("Processdatasize: %d", context->slavelist[slave].Obytes);
    // This is can be used to restore default parameters
    // this->template sdo_write<uint32_t>({ 0x1011, 0x01 }, 0x64616F6C );  // RESTORE PARAMETERS TO DELIVERY STATE
    return 1;
  }

  using current_t = mp_units::quantity<mp_units::si::nano<mp_units::si::ampere>, std::int64_t>;
  static constexpr auto min_ampere = 4'000'000 * current_t::reference;
  static constexpr auto max_ampere = 20'000'000 * current_t::reference;

  void pdo_cycle(const std::array<temporary, size>& input, std::span<std::uint8_t>) noexcept {
    // TODO: compact mode is not enabling, siemens bits look to be applied correctly but this is wasting half of the
    // transmitted size.
    for (auto& raw_value : input) {
      if (!raw_value.value.out_of_range) {
        current_t amps = util::map(raw_value.value.value, 0, 4096, min_ampere, max_ampere);
        using mp_units::si::degree_Celsius;
        [[maybe_unused]] const auto celsius = util::map<current_t, mp_units::quantity<degree_Celsius>>(
            amps, min_ampere, max_ampere, -20 * degree_Celsius, 100 * degree_Celsius);
        [[maybe_unused]] const auto nice_to_print_amps =
            mp_units::value_cast<double>(amps).force_in(mp_units::si::milli<mp_units::si::ampere>);
        // fmt::println("{} {}", nice_to_print_amps, celsius);
        // mp_units::quantity<mp_units::si::degree_Celsius> const amper = util::map(unscaled, 0, , -20 * si::degree_Celsius,
        // 20000 * si::degree_Celsius);
      }
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
  // std::array<siemens_status, size> status_;  // Three status bits
  // std::array<units::isq::si::electric_current<units::isq::si::milliampere>, size> value_;
};

using el3054 = el305x<4, 0xbee3052>;

}  // namespace tfc::ec::devices::beckhoff
