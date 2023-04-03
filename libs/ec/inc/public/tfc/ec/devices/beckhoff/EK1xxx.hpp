#pragma once

#include "tfc/ec/devices/base.hpp"

PRAGMA_CLANG_WARNING_PUSH_OFF(-Wweak-vtables)
namespace tfc::ec::devices::beckhoff {
class ek1100 : public base {
public:
  explicit ek1100(uint16_t slave_index) : base(slave_index) {}
  void process_data(std::span<std::byte>, std::span<std::byte>) final {}
  static constexpr uint32_t product_code = 0x44c2c52;
  static constexpr uint32_t vendor_id = 0x2;
};
}  // namespace tfc::ec::devices::beckhoff
PRAGMA_CLANG_WARNING_POP
