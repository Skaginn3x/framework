#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace tfc::snitch {

// std::int8_t is not in the dbus spec, so we use std::int16_t instead
enum struct level_e : std::int16_t {
  all = -1,
  unknown,
  info,
  warning,
  error,
};

namespace api {

using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
using alarm_id_t = std::uint64_t;
using activation_id_t = alarm_id_t;

// std::int8_t is not in the dbus spec, so we use std::int16_t instead
enum struct active_e : std::int16_t {
  all = -1,
  inactive = 0,
  active = 1
};

struct alarm {
  alarm_id_t alarm_id;
  std::string tfc_id;
  std::string sha1sum;
  level_e lvl{ level_e::unknown };
  bool latching{};
  struct translation {
    std::string description;
    std::string details;
  };
  using locale = std::string;
  std::optional<time_point> registered_at;
  std::unordered_map<locale, translation> translations;
};

struct activation {
  alarm_id_t alarm_id;
  std::uint64_t activation_id;
  std::string description;
  std::string details;
  std::string locale;
  bool active{};
  level_e lvl{ level_e::unknown };
  bool latching{};
  time_point set_timestamp;
  std::optional<time_point> reset_timestamp;
  bool in_requested_locale;
};
namespace dbus {
static constexpr std::string_view service_name = "com.skaginn3x.Alarm";
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
  static constexpr std::string_view try_reset = "TryReset";
  static constexpr std::string_view try_reset_all = "TryResetAll";
}
namespace signals {
  static constexpr std::string_view alarm_activation_changed = "AlarmActivationChanged";
  static constexpr std::string_view try_reset = methods::try_reset;
  static constexpr std::string_view try_reset_all = methods::try_reset_all;
}
}
} // namespace api

} // namespace tfc::snitch
