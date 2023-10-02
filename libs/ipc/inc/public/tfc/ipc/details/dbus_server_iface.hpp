#pragma once

// ipc-ruler.cpp - Dbus API service maintaining a list of signals/slots and which signal
// is connected to which slot

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

namespace tfc::ipc_ruler {
using tfc::ipc::details::type_e;

using dbus_error = tfc::dbus::exception::runtime;

/**
 * A class exposing methods for managing signals and slots
 */
template <typename signal_storage, typename slot_storage>
class ipc_manager {
public:
  using slot_name = std::string_view;
  using signal_name = std::string_view;

  explicit ipc_manager(signal_storage& signals, slot_storage& slots)
      : logger_("ipc_manager"), signals_{ signals }, slots_{ slots } {}

  auto set_callback(std::function<void(slot_name, signal_name)> on_connect_cb) -> void {
    on_connect_cb_ = std::move(on_connect_cb);
  }

  auto register_signal(const std::string_view name, const std::string_view description, type_e type) -> void {
    logger_.trace("register_signal called name: {}, type: {}", name, enum_name(type));
    auto timestamp_now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto str_name = std::string(name);
    auto change_signals = signals_.make_change();
    if (signals_->find(str_name) != signals_->end()) {
      auto it = change_signals->find(str_name);
      it->second.last_registered = timestamp_now;
      it->second.description = std::string(description);
      it->second.type = type;
    } else {
      change_signals->emplace(name, signal{ .name = std::string(name),
                                            .type = type,
                                            .created_by = "",
                                            .created_at = timestamp_now,
                                            .last_registered = timestamp_now,
                                            .description = std::string(description) });
    }
  }

  auto register_slot(const std::string_view name, const std::string_view description, type_e type) -> void {
    logger_.trace("register_slot called name: {}, type: {}", name, enum_name(type));
    auto timestamp_now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto timestamp_never = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>{};

    auto str_name = std::string(name);
    auto change_slots = slots_.make_change();
    if (change_slots->find(str_name) != slots_->end()) {
      auto it = change_slots->find(str_name);
      it->second.last_registered = timestamp_now;
      it->second.description = std::string(description);
      it->second.type = type;
    } else {
      change_slots->emplace(name, slot{ .name = std::string(name),
                                        .type = type,
                                        .created_by = "omar",
                                        .created_at = timestamp_now,
                                        .last_registered = timestamp_now,
                                        .last_modified = timestamp_never,
                                        .modified_by = "",
                                        .connected_to = "",
                                        .description = std::string(description) });
    }
    // Call the connected callback to get the slot connected to its signal if it has one.
    on_connect_cb_(str_name, change_slots->at(str_name).connected_to);
  }

  auto get_all_signals() -> std::vector<signal> {
    logger_.trace("get_all_signals called");
    std::vector<signal> ret;
    ret.reserve(signals_->size());
    std::transform(signals_->begin(), signals_->end(), std::back_inserter(ret),
                   [](const auto& tuple) { return tuple.second; });
    return ret;
  }

  auto get_all_slots() -> std::vector<slot> {
    logger_.trace("get_all_slots called");
    std::vector<slot> ret;
    ret.reserve(slots_->size());
    std::transform(slots_->begin(), slots_->end(), std::back_inserter(ret), [](const auto& tuple) { return tuple.second; });
    return ret;
  }

  auto get_all_connections() -> std::map<std::string, std::vector<std::string>> {
    std::map<std::string, std::vector<std::string>> connections;
    std::for_each(signals_->begin(), signals_->end(),
                  [&](const std::pair<std::string, signal>& key_value) { connections[key_value.second.name] = {}; });
    std::for_each(slots_->begin(), slots_->end(), [&](const std::pair<std::string, slot>& slot) {
      if (!slot.second.connected_to.empty()) {
        connections[slot.second.connected_to].emplace_back(slot.second.name);
      }
    });
    return connections;
  }

  auto connect(const std::string_view slot_name, const std::string_view signal_name) -> void {
    auto str_slot_name = std::string(slot_name);
    auto str_signal_name = std::string(signal_name);
    logger_.trace("connect called, slot: {}, signal: {}", slot_name, signal_name);
    if (slots_->find(str_slot_name) == slots_->end()) {
      std::string const err_msg = fmt::format("Slot ({}) does not exist", slot_name);
      logger_.warn(err_msg);
      throw dbus_error(err_msg);
    }
    if (signals_->find(str_signal_name) == signals_->end()) {
      std::string const err_msg = fmt::format("Signal ({}) does not exist", signal_name);
      logger_.warn(err_msg);
      throw dbus_error(err_msg);
    }
    if (slots_->at(str_slot_name).type != signals_->at(str_signal_name).type) {
      std::string const err_msg = "Signal and slot types dont match";
      logger_.warn(err_msg);
      throw dbus_error(err_msg);
    }

    slots_.make_change()->at(str_slot_name).connected_to = signal_name;
    on_connect_cb_(slot_name, signal_name);
  }

  auto disconnect(const std::string_view slot_name) -> void {
    logger_.trace("disconnect called, slot: {}", slot_name);
    const std::string str_slot_name = std::string(slot_name);
    if (slots_->find(str_slot_name) == slots_->end()) {
      throw std::runtime_error("Slot does not exist");
    }
    slots_.make_change()->at(str_slot_name).connected_to = "";
    on_connect_cb_(slot_name, "");
  }

private:
  tfc::logger::logger logger_;
  signal_storage& signals_;
  slot_storage& slots_;
  std::function<void(std::string_view, std::string_view)> on_connect_cb_;
};

template <typename signal_storage, typename slot_storage>
class ipc_manager_server {
public:
  explicit ipc_manager_server(boost::asio::io_context& ctx,
                              std::unique_ptr<ipc_manager<signal_storage, slot_storage>>&& ipc_manager)
      : ipc_manager_{ std::move(ipc_manager) } {
    connection_ = std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection_);
    connection_->request_name(consts::ipc_ruler_service_name.data());
    dbus_interface_ =
        object_server_->add_unique_interface(consts::ipc_ruler_object_path.data(), consts::ipc_ruler_interface_name.data());

    ipc_manager_->set_callback([&](std::string_view slot_name, std::string_view signal_name) {
      auto message = dbus_interface_->new_signal(consts::connection_change.data());
      message.append(std::tuple<std::string, std::string>(slot_name, signal_name));
      message.signal_send();
    });
    dbus_interface_->register_method(std::string(consts::connect_method),
                                     [&](const std::string& slot_name, const std::string& signal_name) {
                                       ipc_manager_->connect(slot_name, signal_name);
                                       dbus_interface_->signal_property(std::string(consts::slots_property));
                                       dbus_interface_->signal_property(std::string(consts::connections_property));
                                     });

    dbus_interface_->register_method(std::string(consts::disconnect_method), [&](const std::string& slot_name) {
      ipc_manager_->disconnect(slot_name);
      dbus_interface_->signal_property(std::string(consts::slots_property));
      dbus_interface_->signal_property(std::string(consts::connections_property));
    });

    dbus_interface_->register_method(std::string(consts::register_signal),
                                     [&](const std::string& name, const std::string& description, uint8_t type) {
                                       ipc_manager_->register_signal(name, description, static_cast<type_e>(type));
                                       dbus_interface_->signal_property(std::string(consts::signals_property));
                                     });
    dbus_interface_->register_method(std::string(consts::register_slot),
                                     [&](const std::string& name, const std::string& description, uint8_t type) {
                                       ipc_manager_->register_slot(name, description, static_cast<type_e>(type));
                                       dbus_interface_->signal_property(std::string(consts::slots_property));
                                     });

    dbus_interface_->register_property_r<std::string>(
        std::string(consts::signals_property), sdbusplus::vtable::property_::emits_change,
        [&](const auto&) { return glz::write_json(ipc_manager_->get_all_signals()); });

    dbus_interface_->register_property_r<std::string>(
        std::string(consts::slots_property), sdbusplus::vtable::property_::emits_change,
        [&](const auto&) { return glz::write_json(ipc_manager_->get_all_slots()); });

    dbus_interface_->register_property_r<std::string>(
        std::string(consts::connections_property), sdbusplus::vtable::property_::emits_change,
        [&](const auto&) { return glz::write_json(ipc_manager_->get_all_connections()); });

    dbus_interface_->register_signal<std::tuple<std::string, std::string>>("");
    dbus_interface_->initialize();
  }

private:
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;
  std::unique_ptr<ipc_manager<signal_storage, slot_storage>> ipc_manager_;
};

}  // namespace tfc::ipc_ruler
