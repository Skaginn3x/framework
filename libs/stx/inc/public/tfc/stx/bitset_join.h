#pragma once

#include <bitset>
#include <cstdint>

namespace tfc::stx {

template <std::size_t... sizes>
constexpr auto bitset_join(std::bitset<sizes> const&... bitsets) -> std::bitset<(sizes + ...)> {
  static constexpr auto total_size = (sizes + ...);
  auto constexpr append_bitset = [](auto const& bitset, auto& result) {
    result <<= bitset.size();
    result |= std::bitset<total_size>{ bitset.to_ullong() };
  };
  std::bitset<total_size> result;
  (append_bitset(bitsets, result), ...);
  return result;
}

static_assert(bitset_join(std::bitset<1>(0b1), std::bitset<2>(0b10)).size() == 3);
static_assert(bitset_join(std::bitset<3>(0b101), std::bitset<2>(0b10)) == std::bitset<5>(0b10110));
static_assert(bitset_join(std::bitset<9>(0b101001001), std::bitset<3>(0b101), std::bitset<1>(0b0), std::bitset<2>(0b11)) ==
              std::bitset<15>(0b101001001101011));

}  // namespace tfc::stx
