#pragma once

// ipc-ruler.cpp - Dbus API service maintaining a list of signals/slots and which signal
// is connected to which slot

#include <functional>
#include <utility>

#include <boost/asio/io_context.hpp>
#include <magic_enum.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/dbus_util.hpp>
#include <tfc/ipc/enums.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace tfc::ipc_ruler {
// service name
static auto ipc_ruler_service_name = "com.skaginn3x.ipc_ruler";
// object path
static auto ipc_ruler_object_path = "/com/skaginn3x/ipc_ruler";
// Interface name
static auto ipc_ruler_interface_name = "com.skaginn3x.manager";

using dbus_error = tfc::dbus::exception::runtime;

/**
 * A class exposing methods for managing signals and slots
 */
class ipc_manager {
public:
  struct signal {
    std::string name;
    tfc::ipc::type_e type;
    std::string created_by;
    std::string created_at;
    std::string last_registered;
  };
  struct slot {
    std::string name;
    tfc::ipc::type_e type;
    std::string created_by;
    std::string created_at;
    std::string last_registered;
    std::string last_modified;
    std::string modified_by;
    std::string connected_to;
  };

  explicit ipc_manager(std::function<void(std::string, std::string)> on_connect_cb)
      : logger_("ipc_manager"), on_connect_cb_{ std::move(on_connect_cb) } {}

  auto register_signal(std::string name, tfc::ipc::type_e type) -> void {
    logger_.trace("register_signal called name: {}, type: {}", name, magic_enum::enum_name(type));
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
  auto register_slot(const std::string& name, tfc::ipc::type_e type) -> void {
    logger_.trace("register_slot called name: {}, type: {}", name, magic_enum::enum_name(type));
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
  auto get_all_signals() -> std::vector<std::tuple<std::string, uint8_t, std::string, std::string, std::string>> {
    logger_.trace("get_all_signals called");
    std::vector<std::tuple<std::string, uint8_t, std::string, std::string, std::string>> ret;
    for (auto& [_, signal] : signals_) {
      ret.emplace_back(signal.name, static_cast<uint8_t>(signal.type), signal.created_by, signal.created_at,
                       signal.last_registered);
    }
    return ret;
  }
  auto get_all_slots() -> std::vector<
      std::tuple<std::string, uint8_t, std::string, std::string, std::string, std::string, std::string, std::string>> {
    logger_.trace("get_all_slots called");
    std::vector<
        std::tuple<std::string, uint8_t, std::string, std::string, std::string, std::string, std::string, std::string>>
        ret;
    for (auto& [_, slot] : slots_) {
      ret.emplace_back(slot.name, static_cast<uint8_t>(slot.type), slot.created_by, slot.created_at, slot.last_registered,
                       slot.last_modified, slot.modified_by, slot.connected_to);
    }
    return ret;
  }

  auto connect(const std::string& slot_name, const std::string& signal_name) -> void {
    logger_.trace("connect called, slot: {}, signal: {}", slot_name, signal_name);
    if (slots_.find(slot_name) == slots_.end()) {
      std::string const err_msg = fmt::format("Slot ({}) does not exist", slot_name);
      logger_.warn(err_msg);
      throw dbus_error(err_msg);
    }
    if (signals_.find(signal_name) == signals_.end()) {
      std::string const err_msg = fmt::format("Signal ({}) does not exist", signal_name);
      logger_.warn(err_msg);
      throw dbus_error(err_msg);
    }
    if (slots_.at(slot_name).type != signals_.at(signal_name).type) {
      std::string const err_msg = "Signal and slot types dont match";
      logger_.warn(err_msg);
      throw dbus_error(err_msg);
    }

    slots_.at(slot_name).connected_to = signal_name;
    on_connect_cb_(slot_name, signal_name);
    // send a notification
  }
  auto disconnect(const std::string& slot_name) -> void {
    logger_.trace("disconnect called, slot: {}", slot_name);
    if (slots_.find(slot_name) == slots_.end()) {
      throw std::runtime_error("Slot does not exist");
    }
    slots_.at(slot_name).connected_to = "";
    on_connect_cb_(slot_name, "");
  }

private:
  std::unordered_map<std::string, signal> signals_;
  std::unordered_map<std::string, slot> slots_;
  tfc::logger::logger logger_;
  std::function<void(std::string, std::string)> on_connect_cb_;
};

class ipc_manager_server {
public:
  explicit ipc_manager_server(boost::asio::io_context& ctx) {
    // Create a new bus pointer
    sd_bus* bus;
    sd_bus_open_system(&bus);
    connection_ = std::make_shared<sdbusplus::asio::connection>(ctx, bus);
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection_);
    connection_->request_name(ipc_ruler_service_name);
    dbus_iface_ = object_server_->add_unique_interface(ipc_ruler_object_path, ipc_ruler_interface_name);

    ipc_manager_ = std::make_unique<ipc_manager>([&](const std::string& slot_name, const std::string& signal_name) {
      auto message = dbus_iface_->new_signal("connection_change");
      message.append(std::tuple<std::string, std::string>(slot_name, signal_name));
      message.signal_send();
    });

    dbus_iface_->register_method("register_signal", [&](const std::string& name, uint8_t type) {
      ipc_manager_->register_signal(name, static_cast<tfc::ipc::type_e>(type));
    });
    dbus_iface_->register_method("register_slot", [&](const std::string& name, uint8_t type) {
      ipc_manager_->register_slot(name, static_cast<tfc::ipc::type_e>(type));
    });

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
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;
  std::unique_ptr<ipc_manager> ipc_manager_;
};
class ipc_manager_client {
public:
  explicit ipc_manager_client(boost::asio::io_context& ctx) {
    sd_bus* bus;
    sd_bus_open_system(&bus);
    connection_ = std::make_unique<sdbusplus::asio::connection>(ctx, bus);
    match_ = std::make_unique<sdbusplus::bus::match::match>(
        *connection_,
        fmt::format("sender='{}',interface='{}',path='{}',type='signal'", ipc_ruler_service_name, ipc_ruler_interface_name,
                    ipc_ruler_object_path),
        std::bind_front(&ipc_manager_client::match_callback, this));
  }

  template <typename message_handler>
  auto register_signal(const std::string& name, tfc::ipc::type_e type, message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "register_signal", name, static_cast<uint8_t>(type));
  }

  template <typename message_handler>
  auto register_slot(const std::string& name, tfc::ipc::type_e type, message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "register_slot", name, static_cast<uint8_t>(type));
  }

  template <typename message_handler>
  auto get_all_signals(message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "get_all_signals");
  }
  template <typename message_handler>
  auto get_all_slots(message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "get_all_slots");
  }
  template <typename message_handler>
  auto connect(const std::string& slot_name, const std::string& signal_name, message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "connect", slot_name, signal_name);
  }
  template <typename message_handler>
  auto disconnect(const std::string& slot_name, message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "disconnect", slot_name);
  }

  auto match_callback(sdbusplus::message_t& msg) -> void {
    auto container = msg.unpack<std::tuple<std::string, std::string>>();
    connection_change_callback_(std::get<0>(container), std::get<1>(container));
  }

  auto register_connection_change_callback(
      std::function<void(std::string const&, std::string const&)> connection_change_callback) -> void {
    connection_change_callback_ = std::move(connection_change_callback);
  }

private:
  std::unique_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::bus::match::match> match_;
  std::function<void(std::string const&, std::string const&)> connection_change_callback_ = [](std::string const&,
                                                                                               std::string const&) {};
};
}  // namespace tfc::ipc_ruler
