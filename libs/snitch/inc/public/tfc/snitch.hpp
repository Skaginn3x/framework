#pragma once

#include <cstdint>
#include <string_view>

#include <tfc/stx/concepts.hpp>
#include <tfc/snitch/common.hpp>

namespace tfc::snitch {

struct variance {
  bool requires_acknowledgement{};
  level lvl{ level::unknown };
};

template <variance var>
class alarm {
public:
  alarm(std::string_view description, std::string_view details) {}

  template <stx::invocable callback_t>
    requires (var.requires_acknowledgement)
  void on_acknowledgement(callback_t&& callback) {

  }


};

using info = alarm<{ .requires_acknowledgement = false, .lvl = level::info }>;
using warning = alarm<{ .requires_acknowledgement = false, .lvl = level::warning }>;
using warning_latched = alarm<{ .requires_acknowledgement = true, .lvl = level::warning }>;
using warning_ack = warning_latched;
using error = alarm<{ .requires_acknowledgement = false, .lvl = level::error }>;


} // namespace tfc::snitch
