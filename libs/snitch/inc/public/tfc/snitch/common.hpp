#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace tfc::snitch {

enum struct level_e : std::int8_t {
  all = -1,
  unknown,
  info,
  warning,
  error,
};

namespace api {

using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
using alarm_id_t = std::int64_t;

enum struct active_e : std::int8_t {
  all = -1,
  inactive = 0,
  active = 1
};

struct alarm {
  std::uint64_t alarm_id;
  std::string tfc_id;
  std::string sha1sum;
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
  std::uint64_t alarm_id;
  std::uint64_t activation_id;
  std::string description;
  std::string details;
  std::string locale;
  bool active{};
  level_e lvl{ level_e::unknown };
  bool latching{};
  time_point timestamp;
};
namespace dbus {
static constexpr std::string_view interface_name = "com.skaginn3x.Alarm";
static constexpr std::string_view object_path = "/com/skaginn3x/Alarm";
namespace properties {

}
namespace methods {
  static constexpr std::string_view register_alarm = "RegisterAlarm";
  static constexpr std::string_view list_alarms = "ListAlarms";
  static constexpr std::string_view list_activations = "ListActivations";
  static constexpr std::string_view set_alarm = "SetAlarm";
  static constexpr std::string_view reset_alarm = "ResetAlarm";
  static constexpr std::string_view ack_alarm = "AckAlarm";
  static constexpr std::string_view ack_all_alarms = "AckAllAlarms";
}
namespace signals {
  static constexpr std::string_view alarm_changed = "AlarmChanged";
}
}
} // namespace api

} // namespace tfc::snitch
