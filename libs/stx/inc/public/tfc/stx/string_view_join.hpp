#pragma once

#include <array>
#include <string_view>

#define EXPORT __attribute__((visibility("default")))

namespace tfc::stx {

// from https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time

template <std::string_view const&... strs>
struct string_view_join {
  // Join all strings into a single std::array of chars
  static constexpr auto impl() noexcept {
    constexpr auto len = (strs.size() + ... + 0);
    std::array<char, len + 1> array{};
    // clang-format off
    auto append = [idx = 0UL, &array](auto const& view) mutable {
      for (auto character : view) {
        array[idx++] = character;
      }
    };
    // clang-format on
    (append(strs), ...);
    array[len] = 0;
    return array;
  }
  // Give the joined string static storage
  static constexpr auto arr = impl();
  // View as a std::string_view
  static constexpr std::string_view value{ arr.data(), arr.size() - 1 };
};
// Helper to get the value out
template <std::string_view const&... strs>
static constexpr auto string_view_join_v = string_view_join<strs...>::value;

namespace test {
static constexpr std::string_view hello = "hello";
static constexpr std::string_view space = " ";
static constexpr std::string_view world = "world";
static constexpr std::string_view bang = "!";
static_assert(string_view_join_v<hello, space, world, bang> == "hello world!");
}  // namespace test

}  // namespace tfc::stx
