#pragma once

// ipc-ruler.cpp - Dbus API service maintaining a list of signals/slots and which signal
// is connected to which slot

#include <functional>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <magic_enum.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>

#include <tfc/dbus/exception.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ipc/enums.hpp>
#include <tfc/ipc/glaze_meta.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>

namespace tfc::ipc_ruler {
using tfc::ipc::details::type_e;
// service name
static auto ipc_ruler_service_name = "com.skaginn3x.ipc_ruler";
// object path
static auto ipc_ruler_object_path = "/com/skaginn3x/ipc_ruler";
// Interface name
static auto ipc_ruler_interface_name = "com.skaginn3x.manager";

using dbus_error = tfc::dbus::exception::runtime;

struct signal {
  std::string name;
  type_e type;
  std::string created_by;
  std::chrono::time_point<std::chrono::system_clock> created_at;
  std::chrono::time_point<std::chrono::system_clock> last_registered;

  struct glaze {
    static constexpr auto value{ glz::object("name",
                                             &tfc::ipc_ruler::signal::name,
                                             "type",
                                             &tfc::ipc_ruler::signal::type,
                                             "created_by",
                                             &tfc::ipc_ruler::signal::created_by,
                                             "created_at",
                                             &tfc::ipc_ruler::signal::created_at,
                                             "last_registered",
                                             &tfc::ipc_ruler::signal::last_registered) };
    static constexpr auto name{ "signal" };
  };
};

struct slot {
  std::string name;
  type_e type;
  std::string created_by;
  std::chrono::time_point<std::chrono::system_clock> created_at;
  std::chrono::time_point<std::chrono::system_clock> last_registered;
  std::chrono::time_point<std::chrono::system_clock> last_modified;
  std::string modified_by;
  std::string connected_to;

  struct glaze {
    static constexpr auto value{ glz::object("name",
                                             &tfc::ipc_ruler::slot::name,
                                             "type",
                                             &tfc::ipc_ruler::slot::type,
                                             "created_by",
                                             &tfc::ipc_ruler::slot::created_by,
                                             "created_at",
                                             &tfc::ipc_ruler::slot::created_at,
                                             "last_registered",
                                             &tfc::ipc_ruler::slot::last_registered,
                                             "last_modified",
                                             &tfc::ipc_ruler::slot::last_modified,
                                             "modified_by",
                                             &tfc::ipc_ruler::slot::modified_by,
                                             "connected_to",
                                             &tfc::ipc_ruler::slot::connected_to) };
    static constexpr auto name{ "slot" };
  };
};

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

  auto set_callback(std::function<void(slot_name, signal_name)> on_connect_cb) -> void { on_connect_cb_ = on_connect_cb; }
  auto register_signal(const std::string_view name, type_e type) -> void {
    logger_.trace("register_signal called name: {}, type: {}", name, magic_enum::enum_name(type));
    auto timestamp_now = std::chrono::system_clock::now();
    auto str_name = std::string(name);
    if (signals_->find(str_name) != signals_->end()) {
      signals_->at(str_name).last_registered = timestamp_now;
    }
    signals_->emplace(name, signal{ .name = std::string(name),
                                    .type = type,
                                    .created_by = "",
                                    .created_at = timestamp_now,
                                    .last_registered = timestamp_now });
    signals_.set_changed();
  }
  auto register_slot(const std::string_view name, type_e type) -> void {
    logger_.trace("register_slot called name: {}, type: {}", name, magic_enum::enum_name(type));
    auto timestamp_now = std::chrono::system_clock::now();
    auto timestamp_never = std::chrono::time_point<std::chrono::system_clock>{};

    auto str_name = std::string(name);
    if (slots_->find(str_name) != slots_->end()) {
      slots_->at(str_name).last_registered = timestamp_now;
    }
    slots_->emplace(name, slot{
                              .name = std::string(name),
                              .type = type,
                              .created_by = "omar",
                              .created_at = timestamp_now,
                              .last_registered = timestamp_now,
                              .last_modified = timestamp_never,
                              .modified_by = "",
                              .connected_to = "",
                          });
    slots_.set_changed();
  }
  auto get_all_signals() -> std::vector<signal> {
    logger_.trace("get_all_signals called");
    std::vector<signal> ret;
    ret.reserve(signals_->size());
    std::transform(signals_->begin(), signals_->end(), std::back_inserter(ret),
                   [](const auto& tple) { return tple.second; });
    return ret;
  }
  auto get_all_slots() -> std::vector<slot> {
    logger_.trace("get_all_slots called");
    std::vector<slot> ret;
    ret.reserve(slots_->size());
    std::transform(slots_->begin(), slots_->end(), std::back_inserter(ret), [](const auto& tple) { return tple.second; });
    return ret;
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

    slots_->at(str_slot_name).connected_to = signal_name;
    on_connect_cb_(slot_name, signal_name);
    slots_.set_changed();
  }
  auto disconnect(const std::string_view slot_name) -> void {
    logger_.trace("disconnect called, slot: {}", slot_name);
    const std::string str_slot_name = std::string(slot_name);
    if (slots_->find(str_slot_name) == slots_->end()) {
      throw std::runtime_error("Slot does not exist");
    }
    slots_->at(str_slot_name).connected_to = "";
    on_connect_cb_(slot_name, "");
    slots_.set_changed();
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
    connection_->request_name(ipc_ruler_service_name);
    dbus_iface_ = object_server_->add_unique_interface(ipc_ruler_object_path, ipc_ruler_interface_name);

    ipc_manager_->set_callback([&](std::string_view slot_name, std::string_view signal_name) {
      auto message = dbus_iface_->new_signal("connection_change");
      message.append(std::tuple<std::string, std::string>(slot_name, signal_name));
      message.signal_send();
    });

    dbus_iface_->register_method("register_signal", [&](const std::string& name, uint8_t type) {
      ipc_manager_->register_signal(name, static_cast<type_e>(type));
    });
    dbus_iface_->register_method("register_slot", [&](const std::string& name, uint8_t type) {
      ipc_manager_->register_slot(name, static_cast<type_e>(type));
    });

    dbus_iface_->register_property_r<std::string>("signals", sdbusplus::vtable::property_::emits_change, [&](const auto&) {
      return glz::write_json(ipc_manager_->get_all_signals());
    });
    dbus_iface_->register_property_r<std::string>("slots", sdbusplus::vtable::property_::emits_change, [&](const auto&) {
      return glz::write_json(ipc_manager_->get_all_slots());
    });

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
  std::unique_ptr<ipc_manager<signal_storage, slot_storage>> ipc_manager_;
};
class ipc_manager_client {
public:
  explicit ipc_manager_client(boost::asio::io_context& ctx) {
    connection_ = std::make_unique<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());
    match_ = std::make_unique<sdbusplus::bus::match::match>(
        *connection_,
        fmt::format("sender='{}',interface='{}',path='{}',type='signal'", ipc_ruler_service_name, ipc_ruler_interface_name,
                    ipc_ruler_object_path),
        std::bind_front(&ipc_manager_client::match_callback, this));
  }

  template <typename message_handler>
  auto register_signal(const std::string& name, type_e type, message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "register_signal", name, static_cast<uint8_t>(type));
  }

  template <typename message_handler>
  auto register_slot(const std::string_view name, type_e type, message_handler&& handler) -> void {
    connection_->async_method_call(handler, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name,
                                   "register_slot", name, static_cast<uint8_t>(type));
  }

  template <typename message_handler>
  auto signals(message_handler&& handler) -> void {
    sdbusplus::asio::getProperty<std::string>(
        *connection_, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name, "signals",
        [handler](const boost::system::error_code& error, const std::string& response) {
          if (error) {
            return;
          }
          auto signals = glz::read_json<std::vector<signal>>(response);
          if (signals) {
            handler(signals.value());
          }
        });
  }
  template <typename message_handler>
  auto slots(message_handler&& handler) -> void {
    sdbusplus::asio::getProperty<std::string>(
        *connection_, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name, "slots",
        [handler](const boost::system::error_code& error, const std::string& response) {
          if (error) {
            return;
          }
          auto slots = glz::read_json<std::vector<slot>>(response);
          if (slots) {
            handler(slots.value());
          }
        });
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
      std::function<void(std::string_view const, std::string_view const)> connection_change_callback) -> void {
    connection_change_callback_ = std::move(connection_change_callback);
  }

private:
  std::unique_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::bus::match::match> match_;
  std::function<void(std::string_view const, std::string_view const)> connection_change_callback_ =
      [](std::string_view const, std::string_view const) {};
};
}  // namespace tfc::ipc_ruler
