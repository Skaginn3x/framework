#pragma once

#include <cstdint>
#include <string_view>

#include <fmt/core.h>

#include <tfc/stx/concepts.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/snitch/common.hpp>

namespace tfc::snitch {

struct variance {
  bool requires_acknowledgement{};
  level lvl{ level::unknown };
};

template <typename T>
concept named_arg = requires(T t) {
  { t.name } -> std::convertible_to<const char*>;
  { t.value };
};

template <variance var, stx::basic_fixed_string description, stx::basic_fixed_string details>
class alarm {
public:
  /// \example alarm<{}, "short desc", "long desc"> warn(fmt::arg("key", "value"), fmt::arg("key2", 1));
  alarm(named_arg auto&& ... default_args) {
    // todo
  }

  template <stx::invocable callback_t>
    requires (var.requires_acknowledgement)
  void on_acknowledgement(callback_t&& callback) {
  }





};

template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using info = alarm<{ .requires_acknowledgement = false, .lvl = level::info }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning = alarm<{ .requires_acknowledgement = false, .lvl = level::warning }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning_latched = alarm<{ .requires_acknowledgement = true, .lvl = level::warning }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning_ack = warning_latched<description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using error = alarm<{ .requires_acknowledgement = false, .lvl = level::error }, description, details>;


} // namespace tfc::snitch
