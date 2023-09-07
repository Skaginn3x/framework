#pragma once
#include <array>
#include <concepts>
#include <limits>
#include <string_view>
#include <utility>

namespace tfc::stx {

enum struct base_e : std::uint8_t { decimal = 10, hex = 16 };

template <std::unsigned_integral auto integral, base_e base = base_e::decimal>
struct to_string_view {
  template <std::size_t size>
  struct impl_out {
    std::array<char, size> buffer{};
    std::size_t idx{};
  };
  // Join all strings into a single std::array of chars
  static constexpr auto impl() noexcept {
    using integral_t = decltype(integral);
    constexpr auto digits{ []() {
      if constexpr (base == base_e::decimal) {
        // digits10 is count from zero
        return std::numeric_limits<integral_t>::digits10 + 1;
      } else if constexpr (base == base_e::hex) {
        return sizeof(decltype(integral)) * 2;
      }
    } };
    // plus 1 is zero termination, meaning if you would use the outputted data() on the
    // string_view the next byte after the last character is guaranteed to be null
    constexpr int buffer_size = digits() + 1;
    impl_out<buffer_size> result{ .idx = buffer_size - 1 };
    constexpr auto radix = std::to_underlying(base);
    // Lookup table for digit characters
    constexpr std::array<char, 16> digit_chars = { '0', '1', '2', '3', '4', '5', '6', '7',
                                                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    integral_t abs_value = integral;

    do {
      result.buffer[result.idx--] = digit_chars[abs_value % radix];  // Lookup digit character
      abs_value /= radix;
    } while (abs_value != 0);

    return result;
  }
  // Give the joined string static storage
  static constexpr auto res = impl();
  // View as a std::string_view
  static constexpr std::string_view value{ res.buffer.data() + res.idx + 1, res.buffer.size() - res.idx - 1 };
};
template <std::unsigned_integral auto integral, base_e base = base_e::decimal>
static constexpr auto to_string_view_v = to_string_view<integral, base>::value;

namespace test {

using std::string_view_literals::operator""sv;

static_assert(to_string_view_v<10U> == "10"sv);
static_assert(to_string_view_v<0U> == "0"sv);
static_assert(to_string_view_v<65536U> == "65536"sv);
static_assert(to_string_view_v<255U> == "255"sv);
static_assert(to_string_view_v<18446744073709551615UL> == "18446744073709551615"sv);

static_assert(to_string_view_v<15U, base_e::hex> == "F"sv);
static_assert(to_string_view_v<10U, base_e::hex> == "A"sv);
static_assert(to_string_view_v<65536U, base_e::hex> == "10000"sv);
static_assert(to_string_view_v<255U, base_e::hex> == "FF"sv);
static_assert(to_string_view_v<18446744073709551615UL, base_e::hex> == "FFFFFFFFFFFFFFFF"sv);

}  // namespace test

}  // namespace tfc::stx
