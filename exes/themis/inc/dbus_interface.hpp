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
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc/details/dbus_constants.hpp>
#include <tfc/ipc/details/dbus_structs.hpp>
#include <tfc/ipc/details/dbus_structs_glaze_meta.hpp>
#include <tfc/ipc/enums.hpp>
#include <tfc/ipc/glaze_meta.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>


namespace tfc::themis {
using tfc::ipc::details::type_e;

using dbus_error = tfc::dbus::exception::runtime;
using namespace tfc::snitch::api::dbus;

class interface {
public:
  explicit interface(boost::asio::io_context& ctx, tfc::themis::alarm_database& database) {
    connection_ = std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection_);
    connection_->request_name(interface_name.data());
    dbus_interface_ =
        object_server_->add_unique_interface(object_path.data(), interface_name.data());

    dbus_interface_->register_method(std::string(methods::list_alarms),
                                     [&]() -> std::string {
                                       return glz::write_json(database.list_alarms());
                                     });

    dbus_interface_->register_method(std::string(methods::register_alarm),
                                     [&](std::string tfc_id, const std::string& description, const std::string& details, bool latching, int alarm_level) -> std::uint64_t {
                                       // TODO: User supplied alarm_level needs more verification
                                       return database.register_alarm_en(tfc_id, description, details, latching, static_cast<tfc::snitch::level_e>(alarm_level));
                                     });

    dbus_interface_->register_method(std::string(methods::set_alarm),
                                     [&](std::uint64_t alarm_id, const std::unordered_map<std::string, std::string>& args) -> void {
                                       database.set_alarm(alarm_id, args);
                                     });
    dbus_interface_->register_method(std::string(methods::reset_alarm),
                                     [&](std::uint64_t alarm_id) -> void {
                                       database.reset_alarm(alarm_id);
                                     });
    dbus_interface_->register_method(std::string(methods::ack_alarm),
                                     [&](std::uint64_t alarm_id) -> void {
                                       database.ack_alarm(alarm_id);
                                     });
    dbus_interface_->register_method(std::string(methods::ack_all_alarms),
                                     [&]() -> void {
                                       database.ack_all_alarms();
                                     });
    dbus_interface_->register_method(std::string(methods::list_activations),
                                     [&](const std::string& locale, std::uint64_t start_count, std::uint64_t count, int alarm_level, int active, int64_t start, int64_t end) -> std::string {
                                       auto cstart = tfc::themis::alarm_database::timepoint_from_milliseconds(start);
                                       auto cend   = tfc::themis::alarm_database::timepoint_from_milliseconds(end);
                                       return glz::write_json(database.list_activations(locale, start_count, count, static_cast<tfc::snitch::level_e>(alarm_level), static_cast<tfc::snitch::api::active_e>(active), cstart, cend));
                                     });

    // Signal alarm_id, current_activation, ack_status
    dbus_interface_->register_signal<std::tuple<std::uint64_t, bool, bool>>(std::string(signals::alarm_changed));
    dbus_interface_->initialize();
  }

private:
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;
};
}  // namespace tfc::ipc_ruler
