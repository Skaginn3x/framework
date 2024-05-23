#pragma once

#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::beckhoff {
class ek1100 final : public base<ek1100> {
public:
  static constexpr std::string_view name{ "EK1100" };
  explicit ek1100(uint16_t slave_index) : base(slave_index) {}
  void pdo_cycle(std::span<std::uint8_t>, std::span<std::uint8_t>) {}
  static constexpr uint32_t product_code = 0x44c2c52;
  static constexpr uint32_t vendor_id = 0x2;
};
}  // namespace tfc::ec::devices::beckhoff
