#pragma once

#include <concepts>
#include <cstdio>
#include <string_view>
#include <utility>

#include <glaze/util/parse.hpp>

#include <tfc/stx/string_view_join.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::confman {

/// \brief Represent read only parameter.
/// Read only parameter can be written by owning process but not via confman interface.
/// If an request is made to change read_only parameter, the changes are discarded and an
/// error message is printed to stderr.
template <typename value_type>
class read_only {
public:
  using value_t = value_type;

  read_only() = default;

  constexpr explicit read_only(value_type& default_value) : value_{ default_value } {}
  constexpr explicit read_only(value_type&& default_value) : value_{ std::move(default_value) } {}

  auto value() const noexcept -> value_t const& { return value_; }
  auto value() noexcept -> value_t& { return value_; }

  decltype(auto) operator->() noexcept { return std::addressof(value()); }
  decltype(auto) operator->() const noexcept { return std::addressof(value()); }

  friend auto constexpr operator==(read_only const& lhs, read_only const& rhs) noexcept -> bool {
    return lhs.value_ == rhs.value_;
  }
  friend auto constexpr operator==(read_only const& lhs, value_type const& rhs) noexcept -> bool {
    return lhs.value_ == rhs;
  }
  friend auto constexpr operator<=>(read_only const& lhs, read_only const& rhs) noexcept {
    return lhs.value_ <=> rhs.value_;
  }
  friend auto constexpr operator<=>(read_only const& lhs, value_type const& rhs) noexcept { return lhs.value_ <=> rhs; }

private:
  value_t value_{};

public:
  struct glaze {
    static auto constexpr value{ &read_only::value_ };
    static constexpr std::string_view prefix{ "tfc::read_only<" };
    static constexpr std::string_view postfix{ ">" };
    static std::string_view constexpr name{ stx::string_view_join_v<prefix, glz::name_v<value_t>, postfix> };
  };
};

}  // namespace tfc::confman

namespace glz::detail {
template <typename value_t>
struct from_json<tfc::confman::read_only<value_t>> {
  template <auto opts>
  inline static void op(auto&&, is_context auto&& ctx, auto&&... args) noexcept {
    skip_value<opts>(ctx, args...);
  }
};
}  // namespace glz::detail
namespace tfc::json::detail {

template <typename value_t>
struct to_json_schema;

template <typename value_t>
struct to_json_schema<tfc::confman::read_only<value_t>> {
  template <auto opts>
  static void op(auto& schema, auto& defs) noexcept {
    schema.attributes.read_only = true;
    to_json_schema<value_t>::template op<opts>(schema, defs);
  }
};
}  // namespace tfc::json::detail
