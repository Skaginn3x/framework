#pragma once

// ipc-ruler.cpp - Dbus API service maintaining a list of signals/slots and which signal
// is connected to which slot

#include <functional>

#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include "tfc/dbus_util.hpp"
#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"

namespace tfc::ipc_ruler {
// service name
static auto ipc_ruler_service_name = "com.skaginn3x.ipc_ruler";
// object path
static auto ipc_ruler_object_path = "/com/skaginn3x/ipc_ruler";

using dbus_error = tfc::dbus::exception::runtime;

/**
 * A class exposing methods for managing signals and slots
 */
class ipc_manager {
public:
  struct signal {
    std::string name;
    int type;
    std::string created_by;
    std::string created_at;
    std::string last_registered;
  };
  struct slot {
    std::string name;
    int type;
    std::string created_by;
    std::string created_at;
    std::string last_registered;
    std::string last_modified;
    std::string modified_by;
    std::string connected_to;
  };

  ipc_manager(std::function<void(std::string, std::string)> on_connect_cb)
      : logger_("ipc_manager"), on_connect_cb{ on_connect_cb } {}

  auto register_signal(std::string name, int type) -> void {
    logger_.trace("register_signal called name: {}, type: {}", name, type);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto* timestamp_now = std::ctime(&now);
    if (signals_.find(name) != signals_.end()) {
      signals_.at(name).last_registered = timestamp_now;
    }
    signals_.emplace(name, signal{ .name = name,
                                   .type = type,
                                   .created_by = "omar",
                                   .created_at = timestamp_now,
                                   .last_registered = timestamp_now });
  }
  auto register_slot(const std::string& name, int type) -> void {
    logger_.trace("register_slot called name: {}, type: {}", name, type);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto* timestamp_now = std::ctime(&now);

    auto never = std::chrono::system_clock::to_time_t(std::chrono::time_point<std::chrono::system_clock>{});
    auto* timestamp_never = std::ctime(&never);
    if (slots_.find(name) != slots_.end()) {
      slots_.at(name).last_registered = timestamp_now;
    }
    slots_.emplace(name, slot{
                             .name = name,
                             .type = type,
                             .created_by = "omar",
                             .created_at = timestamp_now,
                             .last_registered = timestamp_now,
                             .last_modified = timestamp_never,
                             .modified_by = "",
                             .connected_to = "",
                         });
  }
  auto get_all_signals() -> std::vector<std::tuple<std::string, int, std::string, std::string, std::string>> {
    logger_.trace("get_all_signals called");
    std::vector<std::tuple<std::string, int, std::string, std::string, std::string>> ret;
    for (auto& [_, signal] : signals_) {
      ret.emplace_back(signal.name, signal.type, signal.created_by, signal.created_at, signal.last_registered);
    }
    return ret;
  }
  auto get_all_slots() -> std::vector<
      std::tuple<std::string, int, std::string, std::string, std::string, std::string, std::string, std::string>> {
    logger_.trace("get_all_slots called");
    std::vector<std::tuple<std::string, int, std::string, std::string, std::string, std::string, std::string, std::string>>
        ret;
    for (auto& [_, slot] : slots_) {
      ret.emplace_back(slot.name, slot.type, slot.created_by, slot.created_at, slot.last_registered, slot.last_modified,
                       slot.modified_by, slot.connected_to);
    }
    return ret;
  }

  auto connect(const std::string& slot_name, const std::string& signal_name) -> void {
    logger_.trace("connect called, slot: {}, signal: {}", slot_name, signal_name);
    if (slots_.find(slot_name) == slots_.end()) {
      std::string err_msg = fmt::format("Slot ({}) does not exist", slot_name);
      //logger_.warn(err_msg);
      throw dbus_error(err_msg);
    }
    if (signals_.find(signal_name) == signals_.end()) {
      throw dbus_error(fmt::format("Signal ({}) does not exist", signal_name));
    }
    if (slots_.at(slot_name).type != signals_.at(signal_name).type) {
      throw dbus_error("Signal and slot types dont match");
    }

    slots_.at(slot_name).connected_to = signal_name;
    on_connect_cb(slot_name, signal_name);
    // send a notification
  }
  auto disconnect(const std::string& slot_name) -> void {
    logger_.trace("disconnect called, slot: {}", slot_name);
    if (slots_.find(slot_name) == slots_.end()) {
      throw std::runtime_error("Slot does not exist");
    }
    slots_.at(slot_name).connected_to = "";
    on_connect_cb(slot_name, "");
  }

private:
  std::unordered_map<std::string, signal> signals_;
  std::unordered_map<std::string, slot> slots_;
  tfc::logger::logger logger_;
  std::function<void(std::string, std::string)> on_connect_cb;
};

class ipc_manager_server {
public:
  ipc_manager_server(boost::asio::io_context& ctx)
      : connection_{ std::make_shared<sdbusplus::asio::connection>(ctx) }, object_server_{ connection_ } {
    connection_->request_name(ipc_ruler_service_name);
    dbus_iface_ = object_server_.add_unique_interface(ipc_ruler_object_path, "com.skaginn3x.manager");

    ipc_manager_ = std::make_unique<ipc_manager>([&](std::string slot_name, std::string signal_name) {
      auto message = dbus_iface_->new_signal("connection_change");
      message.append(std::tuple<std::string, std::string>(slot_name, signal_name));
      message.signal_send();
    });

    dbus_iface_->register_method("register_signal",
                                 [&](const std::string& name, int type) { ipc_manager_->register_signal(name, type); });
    dbus_iface_->register_method("register_slot",
                                 [&](const std::string& name, int type) { ipc_manager_->register_slot(name, type); });

    dbus_iface_->register_method("get_all_signals", [&]() { return ipc_manager_->get_all_signals(); });
    dbus_iface_->register_method("get_all_slots", [&]() { return ipc_manager_->get_all_slots(); });

    dbus_iface_->register_method("connect", [&](const std::string& slot_name, const std::string& signal_name) {
      ipc_manager_->connect(slot_name, signal_name);
    });

    dbus_iface_->register_method("disconnect", [&](const std::string& slot_name) { ipc_manager_->disconnect(slot_name); });

    dbus_iface_->register_signal<std::tuple<std::string, std::string>>("connection_change");

    dbus_iface_->initialize();
  }

private:
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::asio::dbus_interface> dbus_iface_;
  sdbusplus::asio::object_server object_server_;
  std::unique_ptr<ipc_manager> ipc_manager_;
};
}  // namespace tfc::ipc_ruler