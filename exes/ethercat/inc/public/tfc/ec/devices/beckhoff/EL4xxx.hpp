#pragma once

#include <tfc/utils/pragmas.hpp>

namespace tfc::ec::devices::beckhoff {
template <size_t size, auto p_code>
class el400x : public base {
public:
  explicit el400x(boost::asio::io_context&, uint16_t const slave_index) : base(slave_index) {}
  static constexpr uint32_t product_code = p_code;
  static constexpr uint32_t vendor_id = 0x2;

  void process_data(std::span<std::byte>, std::span<std::byte> output) noexcept final {
    for (size_t i = 0; i < size; i++) {
      value_[i] += 50;
    }
    point_ = std::chrono::high_resolution_clock::now();

    // Cast pointer type to uint16_t
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wunsafe-buffer-usage)
// clang-format on
    std::span<uint16_t> const output_aligned(reinterpret_cast<uint16_t*>(output.data()), output.size() / 2);
PRAGMA_CLANG_WARNING_POP
    for (size_t i = 0; i < size; i++) {
      output_aligned[i] = value_[i];
    }
  }

private:
  std::array<uint16_t, size> value_;
  std::chrono::time_point<std::chrono::high_resolution_clock> point_ = std::chrono::high_resolution_clock::now();
};

using el4002 = el400x<4, 0xfa23052>;

}  // namespace tfc::ec::devices::beckhoff
