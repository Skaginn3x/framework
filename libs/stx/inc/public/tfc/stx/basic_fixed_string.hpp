#pragma once

#include <algorithm>
#include <cstdint>
#include <string_view>

namespace tfc::stx {

template <typename CharType, unsigned N> struct [[nodiscard]] basic_fixed_string {
  using char_type = CharType;

  char_type data_[N + 1]{};

  [[nodiscard]] constexpr unsigned size() const noexcept { return N; }

  [[nodiscard]] constexpr char_type const *begin() const noexcept { return &data_[0]; }

  [[nodiscard]] constexpr char_type *data() noexcept { return &data_[0]; }

  [[nodiscard]] constexpr char_type const *data() const noexcept { return &data_[0]; }

  [[nodiscard]] constexpr char_type const *end() const noexcept { return &data_[N]; }

  constexpr basic_fixed_string() noexcept : data_{} {}

  template <typename other_char_type>
    requires std::same_as<other_char_type, char_type>
  constexpr basic_fixed_string(const other_char_type (&foo)[N + 1]) noexcept {
    std::copy_n(foo, N + 1, data_);
  }

  template <unsigned other_size>
    requires(N > other_size)
  constexpr basic_fixed_string &operator=(basic_fixed_string<CharType, other_size> &&rhs) noexcept {
    data_ = {0};
    std::move(std::begin(rhs), std::end(rhs), std::begin(data_));
    return *this;
  }

  template <typename other_char_type>
    requires(!std::same_as<other_char_type, char_type>)
  constexpr basic_fixed_string(const other_char_type (&foo)[N + 1]) noexcept {
    std::transform(foo, foo + N + 1, data_, [](other_char_type c) { return static_cast<char_type>(c); });
  }

  constexpr std::basic_string_view<char_type> view() const noexcept { return {&data_[0], N}; }

  constexpr auto operator<=>(basic_fixed_string<char_type, N> const &) const noexcept = default;

  template <unsigned M> constexpr auto operator==(basic_fixed_string<char_type, M> const &r) const noexcept {
    return N == M && view() == r.view();
  }
};

template <typename char_type, unsigned N>
basic_fixed_string(char_type const (&str)[N]) -> basic_fixed_string<char_type, N - 1>;

template <typename char_type, unsigned N, unsigned M>
consteval auto concat_fixed_string(basic_fixed_string<char_type, N> l, basic_fixed_string<char_type, M> r) noexcept {
  basic_fixed_string<char_type, N + M> result;
  auto it{std::copy(l.begin(), l.end(), result.data())};
  it = std::copy(r.begin(), r.end(), it);
  *it = {};
  return result;
}

template <typename char_type, unsigned N, unsigned M, typename... U>
consteval auto concat_fixed_string(basic_fixed_string<char_type, N> l, basic_fixed_string<char_type, M> r, U... u) noexcept {
  return concat_fixed_string(l, concat_fixed_string(r, u...));
}

template <typename char_type, unsigned N, unsigned M>
consteval auto operator+(basic_fixed_string<char_type, N> l, basic_fixed_string<char_type, M> r) noexcept {
  return concat_fixed_string(l, r);
}

template <typename decl_chr_type, typename char_type, unsigned N> consteval auto fs(char_type const (&str)[N]) noexcept {
  return basic_fixed_string<decl_chr_type, N - 1>(str);
}

template <std::size_t N> struct StringLiteral {
  constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

  char value[N];
};

} // namespace tfc::stx
