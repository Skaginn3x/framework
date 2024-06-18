#pragma once

#include <cstdint>
#include <unordered_map>

#include <tfc/snitch/common.hpp>
#include <tfc/dbus/sdbusplus_fwd.hpp>

namespace tfc::snitch::detail {

class dbus_client {
public:
  explicit dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn);

  // todo completion tokens ... std function
  auto register_alarm(std::string_view tfc_id, std::string_view description, std::string_view details, level_e lvl, bool ackable) -> api::alarm_id_t;

  auto list_alarms() -> std::vector<api::alarm>;

  auto list_activations(std::string_view locale, std::uint64_t start_count, std::uint64_t count, level_e, api::active_e, api::time_point start, api::time_point end) -> std::vector<api::activation>;

  auto set_alarm(api::alarm_id_t, std::unordered_map<std::string, std::string> args) -> void;

  auto reset_alarm(api::alarm_id_t) -> void;

  auto ack_alarm(api::alarm_id_t) -> void;

  auto ack_all_alarms() -> void;

  auto match_ack_alarm(api::alarm_id_t, std::function<void()> callback) -> void;
private:
  std::shared_ptr<sdbusplus::asio::connection> dbus_;
};

} // namespace tfc::snitch::detail
