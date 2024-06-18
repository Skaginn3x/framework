#pragma once

#include <cstdint>

namespace tfc::snitch {

enum struct level : std::uint8_t {
  unknown = 0,
  info,
  warning,
  error,
};

namespace api {

struct alarm {
  std::string description;
  std::string details;
  bool active{};
  level lvl{ level::unknown };
  bool latching{};
  struct translation {
    std::string description;
    std::string details;
  };
  using locale = std::string;
  std::unordered_map<locale, translation> translations;
};

struct activation {
  std::string description;
  std::string details;
  bool active{};
  level lvl{ level::unknown };
  bool latching{};
  std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> timestamp;
};

} // namespace api

} // namespace tfc::snitch
