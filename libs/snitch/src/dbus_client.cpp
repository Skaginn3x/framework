
#include <glaze/json.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus/match.hpp>

#include <tfc/dbus/match_rules.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/snitch/details/dbus_client.hpp>
#include <tfc/stx/glaze_meta.hpp>

namespace tfc::snitch::detail {

static constexpr auto try_reset_match_{ dbus::match::rules::make_match_rule<api::dbus::service_name,
                                                                            api::dbus::interface_name,
                                                                            api::dbus::object_path,
                                                                            api::dbus::signals::try_reset,
                                                                            dbus::match::rules::type::signal>() };
static constexpr auto try_reset_all_match_{ dbus::match::rules::make_match_rule<api::dbus::service_name,
                                                                                api::dbus::interface_name,
                                                                                api::dbus::object_path,
                                                                                api::dbus::signals::try_reset_all,
                                                                                dbus::match::rules::type::signal>() };

using std::string_view_literals::operator""sv;
static constexpr std::string_view dbus_service_name = "org.freedesktop.DBus"sv;
static constexpr std::string_view dbus_interface = "org.freedesktop.DBus"sv;
static constexpr std::string_view dbus_path = "/org/freedesktop/DBus"sv;
static constexpr std::string_view dbus_signal = "NameOwnerChanged"sv;
static constexpr std::string_view dbus_arg0 = api::dbus::service_name;
static constexpr std::string_view daemon_alive_match_{ stx::string_view_join_v<dbus::match::rules::type::signal,
                                                                               dbus::match::rules::sender<dbus_service_name>,
                                                                               dbus::match::rules::interface<dbus_interface>,
                                                                               dbus::match::rules::path<dbus_path>,
                                                                               dbus::match::rules::member<dbus_signal>,
                                                                               dbus::match::rules::arg<0, dbus_arg0> > };

dbus_client::dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn) : dbus_{ std::move(conn) } {}
auto dbus_client::register_alarm(std::string_view tfc_id,
                                 std::string_view description,
                                 std::string_view details,
                                 bool resettable,
                                 level_e lvl,
                                 std::function<void(std::error_code const&, api::alarm_id_t)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_,
                           std::string{ api::dbus::methods::register_alarm }  // arguments
                           ,
                           std::string{ tfc_id }, std::string{ description }, std::string{ details }, resettable,
                           std::to_underlying(lvl));
}
auto dbus_client::list_alarms(std::function<void(std::error_code const&, std::vector<api::alarm>)> token) -> void {
  dbus_->async_method_call(
      [token_mv = std::move(token)](std::error_code const& err, std::string buffer) {
        if (err) {
          std::invoke(token_mv, err, std::vector<api::alarm>{});
          return;
        }
        std::vector<api::alarm> result{};
        auto parse_err{ glz::read_json(result, buffer) };
        if (parse_err) {
          // todo convert to error code
          std::invoke(token_mv, std::make_error_code(std::errc::bad_message), std::vector<api::alarm>{});
          return;
        }
        std::invoke(token_mv, err, result);
      },
      service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::list_alarms });
}
auto dbus_client::list_activations(std::string_view locale,
                                   std::uint64_t start_count,
                                   std::uint64_t count,
                                   level_e lvl,
                                   api::active_e active,
                                   api::time_point start,
                                   api::time_point end,
                                   std::function<void(std::error_code const&, std::vector<api::activation>)> token) -> void {
  dbus_->async_method_call(
      [token_mv = std::move(token)](std::error_code const& err, std::string buffer) {
        if (err) {
          std::invoke(token_mv, err, std::vector<api::activation>{});
          return;
        }
        std::vector<api::activation> result{};
        auto parse_err{ glz::read_json(result, buffer) };
        if (parse_err) {
          // todo convert to error code
          std::invoke(token_mv, std::make_error_code(std::errc::bad_message), std::vector<api::activation>{});
          return;
        }
        std::invoke(token_mv, err, result);
      },
      service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::list_activations }  // arguments
      ,
      std::string{ locale }, start_count, count, std::to_underlying(lvl), std::to_underlying(active),
      start.time_since_epoch().count(), end.time_since_epoch().count());
}
auto dbus_client::set_alarm(api::alarm_id_t id,
                            const std::unordered_map<std::string, std::string>& args,
                            std::function<void(std::error_code const&, api::activation_id_t)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_,
                           std::string{ api::dbus::methods::set_alarm }  // arguments
                           ,
                           id, args);
}
auto dbus_client::reset_alarm(api::activation_id_t id, std::function<void(std::error_code const&)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_,
                           std::string{ api::dbus::methods::reset_alarm }  // arguments
                           ,
                           id);
}
auto dbus_client::try_reset_alarm(api::alarm_id_t id, std::function<void(std::error_code const&)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_,
                           std::string{ api::dbus::methods::try_reset }  // arguments
                           ,
                           id);
}
auto dbus_client::try_reset_all_alarms(std::function<void(std::error_code const&)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_,
                           std::string{ api::dbus::methods::try_reset_all });
}
auto dbus_client::on_try_reset_alarm(std::function<void(api::alarm_id_t)> token) -> void {
  try_reset_ = std::make_unique<sdbusplus::bus::match_t>(*dbus_, try_reset_match_.data(),
                                                         [token_mv = std::move(token)](sdbusplus::message_t& msg) {
                                                           api::alarm_id_t id;
                                                           msg.read(id);
                                                           std::invoke(token_mv, id);
                                                         });
}
auto dbus_client::on_try_reset_all_alarms(std::function<void()> token) -> void {
  try_reset_all_ = std::make_unique<sdbusplus::bus::match_t>(
      *dbus_, try_reset_all_match_.data(), [token_mv = std::move(token)](sdbusplus::message_t&) { std::invoke(token_mv); });
}
auto dbus_client::on_daemon_alive(std::function<void()> token) -> void {
  daemon_alive_ = std::make_unique<sdbusplus::bus::match_t>(
      *dbus_, daemon_alive_match_.data(), [token_mv = std::move(token)](sdbusplus::message_t&) { std::invoke(token_mv); });
}

}  // namespace tfc::snitch::detail
