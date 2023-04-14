#pragma once

#include <concepts>
#include <cstdio>
#include <utility>

#include <fmt/core.h>

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

  explicit read_only(value_type& default_value) : value_{ default_value } {}
  explicit read_only(value_type&& default_value) : value_{ std::move(default_value) } {}

  auto value() const noexcept -> value_t const& { return value_; }
  auto value() noexcept -> value_t& { return value_; }

private:
  value_t value_{};
  value_t substitution_{}; // todo differently?

public:
  struct glaze {
    static auto constexpr value{ [](auto&& self) -> auto& {
      fmt::print(stderr, "Trying to set read only parameter\n");
      return self.substitution_;  // returning substitution to write the changes to
    } };
  };
};

}  // namespace tfc::confman
