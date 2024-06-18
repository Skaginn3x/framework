
#include <sdbusplus/asio/connection.hpp>

#include <tfc/snitch/details/dbus_client.hpp>

namespace tfc::snitch::detail {

dbus_client::dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn) : dbus_{ std::move(conn) } {
}

auto dbus_client::register_alarm(std::string_view tfc_id, std::string_view description, std::string_view details, level_e lvl, bool ackable) -> api::alarm_id_t {
  return 0;
}

auto dbus_client::list_alarms() -> std::vector<api::alarm> {
  return {};
}

auto dbus_client::list_activations(std::string_view locale, std::uint64_t start_count, std::uint64_t count, level_e, api::active_e, api::time_point start, api::time_point end) -> std::vector<api::activation> {
  return {};
}

auto dbus_client::set_alarm(api::alarm_id_t, std::unordered_map<std::string, std::string> args) -> void {
}

auto dbus_client::reset_alarm(api::alarm_id_t) -> void {
}

auto dbus_client::ack_alarm(api::alarm_id_t) -> void {
}

auto dbus_client::ack_all_alarms() -> void {
}

} // namespace tfc::snitch::detail
