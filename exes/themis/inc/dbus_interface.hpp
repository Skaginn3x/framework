#pragma once

#include <filesystem>
#include <functional>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <glaze/json.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>
#include <tfc/dbus/match_rules.hpp>
#include <tfc/dbus/exception.hpp>


namespace tfc::themis {
using namespace tfc::snitch::api::dbus;
using std::string_view_literals::operator""sv;
using dbus_error = tfc::dbus::exception::runtime;

class interface {
public:
  explicit interface(std::shared_ptr<sdbusplus::asio::connection> connection, tfc::themis::alarm_database& database) {
    connection_ = connection;
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection_);
    connection_->request_name(service_name.data());
    interface_ =
        object_server_->add_unique_interface(object_path.data(), interface_name.data());

    interface_->register_method(std::string(methods::list_alarms),
                                     [&]() -> std::string {
                                       return glz::write_json(database.list_alarms());
                                     });

    interface_->register_method(std::string(methods::register_alarm),
                                     [&](const sdbusplus::message_t& msg, std::string tfc_id, const std::string& description, const std::string& details, bool latching, std::underlying_type_t<snitch::level_e> alarm_level) -> std::uint64_t {
                                       // TODO: User supplied alarm_level needs more verification
                                       return database.register_alarm_en(tfc_id, description, details, latching, static_cast<tfc::snitch::level_e>(alarm_level));
                                     });

    interface_->register_method(std::string(methods::set_alarm),
                                     [&](snitch::api::alarm_id_t alarm_id, const std::unordered_map<std::string, std::string>& args) -> std::uint64_t {
                                       return database.set_alarm(alarm_id, args);
                                     });
    interface_->register_method(std::string(methods::reset_alarm),
                                     [&](snitch::api::alarm_id_t alarm_id) -> void {
                                       database.reset_alarm(alarm_id);
                                     });
    interface_->register_method(std::string(methods::try_reset),
                                     [&](snitch::api::alarm_id_t alarm_id) -> void {
                                       if (database.is_alarm_active(alarm_id)){
                                         auto message = interface_->new_signal(signals::try_reset.data());
                                         message.append(alarm_id);
                                         message.signal_send();
                                       } else {
                                         throw dbus_error("Alarm is not active");
                                       }
                                     });
    interface_->register_method(std::string(methods::try_reset_all),
                                     [&]() -> void {
                                      if(database.is_some_alarm_active()){
                                        auto message = interface_->new_signal(signals::try_reset_all.data());
                                        message.signal_send();
                                      } else {
                                        throw dbus_error("No alarm is active");
                                      }
                                     });
    using tfc::snitch::api::active_e;
    using tfc::snitch::level_e;
    interface_->register_method(std::string(methods::list_activations),
                                     [&](const std::string& locale, std::uint64_t start_count, std::uint64_t count, std::underlying_type_t<level_e> alarm_level, std::underlying_type_t<active_e> active, int64_t start, int64_t end) -> std::string {
                                       auto cstart = tfc::themis::alarm_database::timepoint_from_milliseconds(start);
                                       auto cend   = tfc::themis::alarm_database::timepoint_from_milliseconds(end);
                                       return glz::write_json(database.list_activations(locale, start_count, count, static_cast<tfc::snitch::level_e>(alarm_level), static_cast<tfc::snitch::api::active_e>(active), cstart, cend));
                                     });

    // Signal alarm_id, current_activation, ack_status
    interface_->register_signal<std::tuple<std::uint64_t, bool, bool>>(std::string(signals::alarm_activation_changed));
    interface_->register_signal<std::uint64_t>(std::string(signals::try_reset));
    interface_->register_signal<void>(std::string(signals::try_reset_all));
    fmt::println(stderr, "{}", match_rule_);
    name_lost_match_ = std::make_unique<sdbusplus::bus::match::match>(
        *connection_, match_rule_.data(), std::bind_front(&interface::match_callback, this));
    interface_->initialize();

  }

private:
  static constexpr std::string_view dbus_interface_ = "org.freedesktop.DBus"sv;
  static constexpr std::string_view dbus_name_ = "org.freedesktop.DBus"sv;
  static constexpr std::string_view dbus_path_ = "/org/freedesktop/DBus"sv;
  static constexpr std::string_view name_owner_changed_ = "NameOwnerChanged"sv;
  static constexpr std::string_view match_rule_ = tfc::dbus::match::rules::make_match_rule<dbus_name_, dbus_interface_, dbus_path_, name_owner_changed_, tfc::dbus::match::rules::type::signal>();
  auto match_callback(sdbusplus::message_t& msg) -> void {
    std::string name;
    std::string old_owner;
    std::string new_owner;
    msg.read(name, old_owner, new_owner);
    if (name == old_owner){
      // Someone has left the chat.
      auto iterator = dbus_ids_to_monitor_.find(name);
      if (iterator != dbus_ids_to_monitor_.end()){
        // We knew that guy
        //TODO: Decide what to do here.
        //TODO: Could cleanup the alarm state, and place a warning into the logs that this process is not handling his alarms anymore.
        //TODO: Could also just ignore it, and let the alarm state be as it is.
        //TODO: There is also the question of alarms that are active when the system is shut down. Who cleans them up?
        fmt::println(stderr, "name: {}, old_owner: {}, new_owner: {}", name, old_owner, new_owner);
      }
    }
  }
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::asio::dbus_interface> interface_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;
  std::unique_ptr<sdbusplus::bus::match::match> name_lost_match_;
  std::unordered_map<std::string, std::vector<uint64_t>> dbus_ids_to_monitor_; // Alarm members and monitoring parties
};
}  // namespace tfc::ipc_ruler
