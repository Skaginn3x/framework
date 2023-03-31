#pragma once
#include <array>
#include <concepts>
#include <limits>
#include <string_view>

namespace tfc::stx {

template <std::unsigned_integral auto integral>
struct to_string_view {
  template <std::size_t size>
  struct impl_out {
    std::array<char, size> buffer{};
    std::size_t idx{};
  };
  // Join all strings into a single std::array of chars
  static constexpr auto impl() noexcept {
    using integral_t = decltype(integral);
    constexpr int buffer_size = std::numeric_limits<integral_t>::digits10 + 1;  // Add 1 for null terminator
    impl_out<buffer_size> result{ .idx = buffer_size - 1 };
    constexpr int radix = 10;  // Base 10 for decimal representation
    // Lookup table for digit characters
    constexpr std::array<char, 10> digit_chars = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

    integral_t abs_value = integral;

    do {
      result.buffer[--result.idx] = digit_chars[abs_value % radix];  // Lookup digit character
      abs_value /= radix;
    } while (abs_value != 0);

    return result;
  }
  // Give the joined string static storage
  static constexpr auto res = impl();
  // View as a std::string_view
  static constexpr std::string_view value{ res.buffer.data() + res.idx, res.buffer.size() - res.idx - 1 };
};
template <std::unsigned_integral auto integral>
static constexpr auto to_string_view_v = to_string_view<integral>::value;

namespace test {

using std::string_view_literals::operator""sv;

static_assert(to_string_view_v<10U> == "10"sv);
static_assert(to_string_view_v<0U> == "0"sv);
static_assert(to_string_view_v<65536U> == "65536"sv);

}  // namespace test

}  // namespace tfc::stx
