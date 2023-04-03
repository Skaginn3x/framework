#pragma once

#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::beckhoff {
class ek1100 final : public base {
public:
  ~ek1100() final;
  explicit ek1100(uint16_t slave_index) : base(slave_index) {}
  void process_data(std::span<std::byte>, std::span<std::byte>) final {}
  static constexpr uint32_t product_code = 0x44c2c52;
  static constexpr uint32_t vendor_id = 0x2;
};
}  // namespace tfc::ec::devices::beckhoff
