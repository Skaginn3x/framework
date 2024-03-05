#pragma once

#include "tfc/ec/devices/base.hpp"
#include "i550/pdo.hpp"

namespace tfc::ec::devices::lenze::i550 {
class i550 final : public base {
public:
  ~i550() final {}
  explicit i550(uint16_t slave_index) : base(slave_index) {}
  void process_data(std::span<std::byte> input, std::span<std::byte> output) final {
    // if (input.size() != sizeof(input_t)) {
    //   logger_.error("Input data size mismatch: expected {}, got {}", sizeof(input_t), input.size());
    //   return;
    // }
  }
  static constexpr uint32_t product_code = 0x69055000;
  static constexpr uint32_t vendor_id = 0x3b;
};
}  // namespace tfc::ec::devices::lenze
