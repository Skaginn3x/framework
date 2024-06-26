#pragma once

#include <filesystem>
#include <functional>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <glaze/json.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>
#include <tfc/dbus/exception.hpp>
#include <tfc/dbus/match_rules.hpp>
#include <tfc/stx/glaze_meta.hpp>

namespace tfc::themis {
using namespace tfc::snitch::api::dbus;
using std::string_view_literals::operator""sv;
using dbus_error = tfc::dbus::exception::runtime;

using tfc::snitch::level_e;
using tfc::snitch::api::alarm_id_t;
using tfc::snitch::api::state_e;

class interface {
public:
  explicit interface(std::shared_ptr<sdbusplus::asio::connection> connection, tfc::themis::alarm_database& database)
      : connection_(std::move(connection)), database_(database) {
    // Initialize the object server and request the service name
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection_);
    connection_->request_name(service_name.data());
    interface_ = object_server_->add_unique_interface(object_path.data(), interface_name.data());

    interface_->register_method(std::string(methods::list_alarms), [&]() -> std::string {
      auto const alarms_str{ glz::write_json(database.list_alarms()) };
      if (!alarms_str) {
        throw dbus_error("Failed to serialize alarms");
      }
      return alarms_str.value();
    });

    interface_->register_method(
        std::string(methods::register_alarm),
        [&](const sdbusplus::message_t& msg, std::string tfc_id, const std::string& description, const std::string& details,
            bool latching, std::underlying_type_t<snitch::level_e> alarm_level) -> std::uint64_t {
          std::string sender = msg.get_sender();
          if (dbus_ids_to_monitor_.find(sender) == dbus_ids_to_monitor_.end()) {
            dbus_ids_to_monitor_[sender] = {};
          }
          auto alarm_id = database.register_alarm_en(tfc_id, description, details, latching,
                                                     static_cast<tfc::snitch::level_e>(alarm_level));
          dbus_ids_to_monitor_[sender].push_back(alarm_id);
          notify_alarm_state(alarm_id, state_e::inactive);
          return alarm_id;
        });

    interface_->register_method(
        std::string(methods::set_alarm),
        [&](snitch::api::alarm_id_t alarm_id, const std::unordered_map<std::string, std::string>& args) -> std::uint64_t {
          auto activation_id = database.set_alarm(alarm_id, args);
          notify_alarm_state(alarm_id, state_e::active);
          return activation_id;
        });
    interface_->register_method(std::string(methods::reset_alarm), [&](snitch::api::alarm_id_t alarm_id) -> void {
      if (!database.reset_alarm(alarm_id)) {
        throw dbus_error("Cannot reset an inactive activation");
      } else {
        // Notify the change in alarm status of this alarm
        notify_alarm_state(alarm_id, state_e::inactive);
      }
    });

    interface_->register_method(std::string(methods::try_reset), [&](snitch::api::alarm_id_t alarm_id) -> void {
      if (database.is_alarm_active(alarm_id)) {
        auto message = interface_->new_signal(signals::try_reset.data());
        message.append(alarm_id);
        message.signal_send();
      } else {
        throw dbus_error("Alarm is not active");
      }
    });

    interface_->register_method(std::string(methods::try_reset_all), [&]() -> void {
      if (database.is_some_alarm_active()) {
        auto message = interface_->new_signal(signals::try_reset_all.data());
        message.signal_send();
      } else {
        throw dbus_error("No alarm is active");
      }
    });

    interface_->register_method(std::string(methods::list_activations),
                                [&](const std::string& locale, std::uint64_t start_count, std::uint64_t count,
                                    std::underlying_type_t<level_e> alarm_level, std::underlying_type_t<state_e> active,
                                    int64_t start, int64_t end) -> std::string {
                                  auto cstart = tfc::themis::alarm_database::timepoint_from_milliseconds(start);
                                  auto cend = tfc::themis::alarm_database::timepoint_from_milliseconds(end);
                                  auto const activations_str{ glz::write_json(database.list_activations(
                                      locale, start_count, count, static_cast<tfc::snitch::level_e>(alarm_level),
                                      static_cast<tfc::snitch::api::state_e>(active), cstart, cend)) };
                                  if (!activations_str) {
                                    throw dbus_error("Failed to serialize activations");
                                  }
                                  return activations_str.value();
                                });

    // Signal alarm_id, current_activation, ack_status
    interface_->register_signal<std::tuple<tfc::snitch::api::alarm_id_t, std::underlying_type_t<tfc::snitch::api::state_e>>>(
        std::string(signals::alarm_activation_changed));
    interface_->register_signal<std::uint64_t>(std::string(signals::try_reset));
    interface_->register_signal<void>(std::string(signals::try_reset_all));
    name_lost_match_ = std::make_unique<sdbusplus::bus::match::match>(*connection_, match_rule_.data(),
                                                                      std::bind_front(&interface::match_callback, this));
    interface_->initialize();
  }

private:
  static constexpr std::string_view dbus_interface_ = "org.freedesktop.DBus"sv;
  static constexpr std::string_view dbus_name_ = "org.freedesktop.DBus"sv;
  static constexpr std::string_view dbus_path_ = "/org/freedesktop/DBus"sv;
  static constexpr std::string_view name_owner_changed_ = "NameOwnerChanged"sv;
  static constexpr std::string_view match_rule_ = tfc::dbus::match::rules::
      make_match_rule<dbus_name_, dbus_interface_, dbus_path_, name_owner_changed_, tfc::dbus::match::rules::type::signal>();

  auto notify_alarm_state(alarm_id_t alarm_id, state_e state) -> void {
    sdbusplus::message_t alarm_change_message = interface_->new_signal(signals::alarm_activation_changed.data());
    alarm_change_message.append(std::tuple(alarm_id, std::to_underlying(state)));
    alarm_change_message.signal_send();
  }
  auto match_callback(sdbusplus::message_t& msg) -> void {
    std::tuple<std::string, std::string, std::string> container;
    sdbusplus::utility::read_into_tuple(container, msg);
    auto [name, old_owner, new_owner] = container;
    if (name == old_owner) {
      auto iterator = dbus_ids_to_monitor_.find(name);
      if (iterator != dbus_ids_to_monitor_.end()) {
        // Someone who reported an alarm to us has gone away. Set all their active alarms to unknown
        // status
        for (auto& [peer, alarm_vec] : dbus_ids_to_monitor_) {
          for (auto& alarm_id : alarm_vec) {
            auto activation = database_.get_activation_id_for_active_alarm(alarm_id);
            if (activation.has_value()) {
              database_.set_activation_status(activation.value(), tfc::snitch::api::state_e::unknown);
              notify_alarm_state(alarm_id, state_e::unknown);
            }
          }
        }
        dbus_ids_to_monitor_.erase(iterator);
      }
    }
  }
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::asio::dbus_interface> interface_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;
  std::unique_ptr<sdbusplus::bus::match::match> name_lost_match_;
  std::unordered_map<std::string, std::vector<tfc::snitch::api::alarm_id_t>>
      dbus_ids_to_monitor_;  // Alarm members and monitoring parties
  alarm_database& database_;
};
}  // namespace tfc::themis
