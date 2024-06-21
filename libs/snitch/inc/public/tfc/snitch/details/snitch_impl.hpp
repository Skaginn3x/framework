#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

#include <boost/asio/steady_timer.hpp>

#include <tfc/snitch/common.hpp>
#include <tfc/dbus/sdbusplus_fwd.hpp>

namespace tfc::logger {
class logger;
}  // namespace tfc::logger

namespace tfc::snitch::detail {

namespace asio = boost::asio;

class dbus_client;

class alarm_impl {
public:
  alarm_impl(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view unique_id, std::string_view description, std::string_view details, bool resettable, level_e lvl, std::unordered_map<std::string, std::string>&& default_args);
  alarm_impl(alarm_impl const&) = delete;
  auto operator=(alarm_impl const&) -> alarm_impl& = delete;
  alarm_impl(alarm_impl&&) = delete;
  auto operator=(alarm_impl&&) -> alarm_impl& = delete;
  ~alarm_impl() = default;

  auto const& default_values() const noexcept { return default_values_; }
  void on_try_reset(std::function<void()> callback);
  void set(std::string_view description_formatted, std::string_view details_formatted, std::unordered_map<std::string, std::string>&& args, std::function<void(std::error_code)>&& on_set_finished);
  void reset();

private:
  void register_alarm();
  void set(std::unordered_map<std::string, std::string>&& args, std::function<void(std::error_code)>&& on_set_finished);
  std::optional<api::activation_id_t> activation_id_{};
  std::optional<api::alarm_id_t> alarm_id_{};
  std::string given_id_;
  std::string description_;
  std::string details_;
  bool resettable_{};
  level_e lvl_{ level_e::unknown };
  std::string tfc_id_;
  std::unordered_map<std::string, std::string> default_values_;
  std::shared_ptr<sdbusplus::asio::connection> conn_;
  std::unique_ptr<dbus_client, std::function<void(dbus_client*)>> dbus_client_;
  std::unique_ptr<logger::logger, std::function<void(logger::logger*)>> logger_;
  asio::steady_timer retry_timer_;
};

} // namespace tfc::snitch::detail
