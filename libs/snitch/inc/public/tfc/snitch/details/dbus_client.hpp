#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/snitch/common.hpp>

namespace tfc::snitch::detail {

class dbus_client {
public:
  explicit dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn);

  auto register_alarm(std::string_view tfc_id,
                      std::string_view description,
                      std::string_view details,
                      bool resettable,
                      level_e lvl,
                      std::function<void(std::error_code const&, api::alarm_id_t)> token) -> void;

  auto list_alarms(std::function<void(std::error_code const&, std::vector<api::alarm>)> token) -> void;

  auto list_activations(std::string_view locale,
                        std::uint64_t start_count,
                        std::uint64_t count,
                        level_e,
                        api::state_e,
                        api::time_point start,
                        api::time_point end,
                        std::function<void(std::error_code const&, std::vector<api::activation>)> token) -> void;

  auto set_alarm(
      api::alarm_id_t,
      const std::unordered_map<std::string, std::string>& args,
      std::function<void(std::error_code const&, api::activation_id_t)> token = [](auto const&, auto) {}) -> void;

  auto reset_alarm(
      api::activation_id_t,
      std::function<void(std::error_code const&)> token = [](auto const&) {}) -> void;

  auto try_reset_alarm(api::alarm_id_t, std::function<void(std::error_code const&)>) -> void;

  auto try_reset_all_alarms(std::function<void(std::error_code const&)>) -> void;

  auto on_try_reset_alarm(std::function<void(api::alarm_id_t)>) -> void;

  auto on_try_reset_all_alarms(std::function<void()>) -> void;

  auto on_daemon_alive(std::function<void()>) -> void;

private:
  std::shared_ptr<sdbusplus::asio::connection> dbus_;
  std::unique_ptr<sdbusplus::bus::match::match, std::function<void(sdbusplus::bus::match::match*)>> try_reset_;
  std::unique_ptr<sdbusplus::bus::match::match, std::function<void(sdbusplus::bus::match::match*)>> try_reset_all_;
  std::unique_ptr<sdbusplus::bus::match::match, std::function<void(sdbusplus::bus::match::match*)>> daemon_alive_;
  const std::string service_name_{ api::dbus::service_name };
  const std::string interface_name_{ api::dbus::interface_name };
  const std::string object_path_{ api::dbus::object_path };
};

}  // namespace tfc::snitch::detail
