#pragma once

#include <array>
#include <chrono>
#include <cstdint>

#include <glaze/util/string_literal.hpp>

#include "tfc/stx/to_string_view.hpp"

namespace tfc::ec::devices::beckhoff {
template <std::size_t size, auto p_code>
class el400x final : public base<el400x<size, p_code>> {
public:
  explicit el400x(boost::asio::io_context&, std::uint16_t const slave_index) : base<el400x>(slave_index) {}
  static constexpr std::uint32_t product_code = p_code;
  static constexpr std::uint32_t vendor_id = 0x2;
  using output_pdo = std::array<std::uint16_t, size>;
  static constexpr std::string_view name{ glz::join_v<glz::chars<"EL400">, stx::to_string_view_v<size>> };

  void pdo_cycle(std::span<std::uint8_t>, [[maybe_unused]] output_pdo& output) noexcept {
    for (size_t i = 0; i < size; i++) {
      value_[i] += 50;
    }
    point_ = std::chrono::high_resolution_clock::now();

    // for (size_t i = 0; i < size; i++) {
    //   output_aligned[i] = value_[i];
    // }
  }

private:
  std::array<std::uint16_t, size> value_;
  std::chrono::time_point<std::chrono::high_resolution_clock> point_ = std::chrono::high_resolution_clock::now();
};

using el4002 = el400x<4, 0xfa23052>;

}  // namespace tfc::ec::devices::beckhoff
