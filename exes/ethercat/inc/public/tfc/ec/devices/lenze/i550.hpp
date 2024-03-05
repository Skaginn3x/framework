#pragma once

#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::lenze {
class i550 final : public base {
public:
  ~i550() final;
  explicit i550(uint16_t slave_index) : base(slave_index) {}
  void process_data(std::span<std::byte>, std::span<std::byte>) final {}
  static constexpr uint32_t product_code = 0x69055000;
  static constexpr uint32_t vendor_id = 0x3b;
};
}  // namespace tfc::ec::devices::lenze
