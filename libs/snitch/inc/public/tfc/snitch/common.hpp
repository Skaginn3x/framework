#pragma once

#include <cstdint>

namespace tfc::snitch {

enum struct level_e : std::uint8_t {
  unknown = 0,
  info,
  warning,
  error,
};

namespace api {

using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
using alarm_id_t = std::int64_t;

enum struct active_e : std::int32_t {
  all = -1,
  inactive = 0,
  active = 1
};

struct alarm {
  std::string description;
  std::string details;
  bool active{};
  level_e lvl{ level_e::unknown };
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
  level_e lvl{ level_e::unknown };
  bool latching{};
  time_point timestamp;
};

} // namespace api

} // namespace tfc::snitch
