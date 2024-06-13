#pragma once

#include <cstdint>

namespace tfc::snitch {

enum struct level : std::uint8_t {
  unknown = 0,
  info,
  warning,
  error,
};

} // namespace tfc::snitch
