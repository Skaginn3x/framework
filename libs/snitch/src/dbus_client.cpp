
#include <sdbusplus/asio/connection.hpp>
#include <glaze/json.hpp>

#include <tfc/stx/glaze_meta.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/snitch/details/dbus_client.hpp>

namespace tfc::snitch::detail {

dbus_client::dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn) : dbus_{ std::move(conn) } {}
auto dbus_client::register_alarm(std::string_view tfc_id,
                                 std::string_view description,
                                 std::string_view details,
                                 bool resettable,
                                 level_e lvl,
                                 std::function<void(std::error_code const&, api::alarm_id_t)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::register_alarm }
    // arguments
    , std::string{ tfc_id }, std::string{ description }, std::string{ details }, resettable, std::to_underlying(lvl));
}
auto dbus_client::list_alarms(std::function<void(std::error_code const&, std::vector<api::alarm>)> token) -> void {
  dbus_->async_method_call([token_mv = std::move(token)](std::error_code const& err, std::string buffer) {
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
  }, service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::list_alarms });
}
auto dbus_client::list_activations(std::string_view locale,
                                   std::uint64_t start_count,
                                   std::uint64_t count,
                                   level_e lvl,
                                   api::active_e active,
                                   api::time_point start,
                                   api::time_point end,
                                   std::function<void(std::error_code const&, std::vector<api::activation>)> token) -> void {
  dbus_->async_method_call([token_mv = std::move(token)](std::error_code const& err, std::string buffer) {
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
  }, service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::list_activations }
    // arguments
    , std::string{ locale }, start_count, count, std::to_underlying(lvl), std::to_underlying(active), start.time_since_epoch().count(), end.time_since_epoch().count()
    );
}
auto dbus_client::set_alarm(api::alarm_id_t id, const std::unordered_map<std::string, std::string> & args, std::function<void(std::error_code const&)> token)
    -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::set_alarm }
    // arguments
    , id, args);
}
auto dbus_client::reset_alarm(api::alarm_id_t id, std::function<void(std::error_code const&)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::reset_alarm }
    // arguments
    , id);
}
auto dbus_client::try_reset_alarm(api::alarm_id_t id, std::function<void(std::error_code const&)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::try_reset }
    // arguments
    , id);
}
auto dbus_client::try_reset_all_alarms(std::function<void(std::error_code const&)> token) -> void {
  dbus_->async_method_call(std::move(token), service_name_, object_path_, interface_name_, std::string{ api::dbus::methods::try_reset_all });
}

} // namespace tfc::snitch::detail
