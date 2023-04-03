#pragma once

#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::beckhoff {
class ek1100 : public base {
public:
  explicit ek1100(uint16_t slave_index) : base(slave_index) {}
  static constexpr uint32_t product_code = 0x44c2c52;
  static constexpr uint32_t vendor_id = 0x2;
};
}  // namespace tfc::ec::devices::beckhoff
