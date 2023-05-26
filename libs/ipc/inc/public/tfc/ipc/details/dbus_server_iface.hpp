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
  std::string description;

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
                                             &tfc::ipc_ruler::signal::last_registered,
                                             "description",
                                             &tfc::ipc_ruler::signal::description) };
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
  std::string description;

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
                                             &tfc::ipc_ruler::slot::connected_to,
                                             "description",
                                             &tfc::ipc_ruler::slot::description) };
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

  auto set_callback(std::function<void(slot_name, signal_name)> on_connect_cb) -> void {
    on_connect_cb_ = std::move(on_connect_cb);
  }

  auto register_signal(const std::string_view name, const std::string_view description, type_e type) -> void {
    logger_.trace("register_signal called name: {}, type: {}", name, magic_enum::enum_name(type));
    auto timestamp_now = std::chrono::system_clock::now();
    auto str_name = std::string(name);
    auto change_signals = signals_.make_change();
    if (signals_->find(str_name) != signals_->end()) {
      change_signals->at(str_name).last_registered = timestamp_now;
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
    logger_.trace("register_slot called name: {}, type: {}", name, magic_enum::enum_name(type));
    auto timestamp_now = std::chrono::system_clock::now();
    auto timestamp_never = std::chrono::time_point<std::chrono::system_clock>{};

    auto str_name = std::string(name);
    auto change_slots = slots_.make_change();
    if (change_slots->find(str_name) != slots_->end()) {
      change_slots->at(str_name).last_registered = timestamp_now;
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

  auto get_all_connections() -> std::map<std::string, std::vector<std::string>> {
    std::map<std::string, std::vector<std::string>> connections;
    std::for_each(signals_->begin(), signals_->end(),
                  [&](std::pair<std::string, signal> kv) { connections[kv.second.name] = {}; });
    std::for_each(slots_->begin(), slots_->end(), [&](std::pair<std::string, slot> kv) {
      if (kv.second.connected_to != "") {
        connections[kv.second.connected_to].emplace_back(kv.second.name);
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
                              std::unique_ptr<ipc_manager<signal_storage, slot_storage>>

                                  && ipc_manager)
      : ipc_manager_{ std::move(ipc_manager) } {
    connection_ = std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection_);
    connection_->request_name(ipc_ruler_service_name);
    dbus_iface_ = object_server_->add_unique_interface(ipc_ruler_object_path, ipc_ruler_interface_name);

    ipc_manager_->set_callback([&](std::string_view slot_name, std::string_view signal_name) {
      auto message = dbus_iface_->new_signal("ConnectionChange");
      message.append(std::tuple<std::string, std::string>(slot_name, signal_name));
      message.signal_send();
    });
    dbus_iface_->register_method("Connect", [&](const std::string& slot_name, const std::string& signal_name) {
      ipc_manager_->connect(slot_name, signal_name);
    });

    dbus_iface_->register_method("Disconnect", [&](const std::string& slot_name) { ipc_manager_->disconnect(slot_name); });

    dbus_iface_->register_method("RegisterSignal",
                                 [&](const std::string& name, const std::string& description, uint8_t type) {
                                   ipc_manager_->register_signal(name, description, static_cast<type_e>(type));
                                 });
    dbus_iface_->register_method("RegisterSlot", [&](const std::string& name, const std::string& description, uint8_t type) {
      ipc_manager_->register_slot(name, description, static_cast<type_e>(type));
    });

    dbus_iface_->register_property_r<std::string>("Signals", sdbusplus::vtable::property_::emits_change, [&](const auto&) {
      return glz::write_json(ipc_manager_->get_all_signals());
    });

    dbus_iface_->register_property_r<std::string>("Slots", sdbusplus::vtable::property_::emits_change, [&](const auto&) {
      return glz::write_json(ipc_manager_->get_all_slots());
    });

    dbus_iface_->register_property_r<std::string>(
        "Connections", sdbusplus::vtable::property_::emits_change,
        [&](const auto&) { return glz::write_json(ipc_manager_->get_all_connections()); });

    dbus_iface_->register_signal<std::tuple<std::string, std::string>>("ConnectionChange");
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

  /**
   * Register a signal with the ipc_manager service running on dbus
   * @param name the name of the signal to be registered
   * @param type  the type enum of the signal to be registered
   * @param handler  the error handling callback function
   */
  auto register_signal(const std::string& name,
                       const std::string_view description,
                       type_e type,
                       std::invocable<const std::error_code&> auto&& handler) -> void {
    connection_->async_method_call(std::forward<decltype(handler)>(handler), ipc_ruler_service_name, ipc_ruler_object_path,
                                   ipc_ruler_interface_name, "RegisterSignal", name, description,
                                   static_cast<uint8_t>(type));
  }

  /**
   * Register a slot with the ipc_manager service running on dbus
   * @param name the name of the slot to be registered
   * @param type  the type enum of the slot to be registered
   * @param handler  the error handling callback function
   */
  auto register_slot(const std::string_view name,
                     const std::string_view description,
                     type_e type,
                     std::invocable<const std::error_code&> auto&& handler) -> void {
    connection_->async_method_call(std::forward<decltype(handler)>(handler), ipc_ruler_service_name, ipc_ruler_object_path,
                                   ipc_ruler_interface_name, "RegisterSlot", name, description, static_cast<uint8_t>(type));
  }

  /**
   * Async function to get the signals property from the ipc manager
   * This fetches the signals over dbus and then calls the provided callback with
   * a vector of signals
   * @param handler a function like object that is called back with a vector of signals
   */
  auto signals(std::invocable<const std::vector<signal>&> auto&& handler) -> void {
    sdbusplus::asio::getProperty<std::string>(*connection_, ipc_ruler_service_name, ipc_ruler_object_path,
                                              ipc_ruler_interface_name, "Signals",
                                              [handler = std::forward<decltype(handler)>(handler)](
                                                  const boost::system::error_code& error, const std::string& response) {
                                                if (error) {
                                                  return;
                                                }
                                                auto signals = glz::read_json<std::vector<signal>>(response);
                                                if (signals) {
                                                  handler(signals.value());
                                                }
                                              });
  }

  /**
   * Async function to get the slots property from the ipc manager
   * This fetches the slots over dbus and then calls the provided callback with
   * a vector of slots
   * @param handler a function like object that is called back with a vector of slots
   */
  auto slots(std::invocable<const std::vector<slot>&> auto&& handler) -> void {
    sdbusplus::asio::getProperty<std::string>(*connection_, ipc_ruler_service_name, ipc_ruler_object_path,
                                              ipc_ruler_interface_name, "Slots",
                                              [handler = std::forward<decltype(handler)>(handler)](
                                                  const boost::system::error_code& error, const std::string& response) {
                                                if (error) {
                                                  return;
                                                }
                                                auto slots = glz::read_json<std::vector<slot>>(response);
                                                if (slots) {
                                                  handler(slots.value());
                                                }
                                              });
  }
  /**
   * Async function to get the connections from the ipc manager.
   * @param handler  a function like object that is called with a map og strings and a vector of strings
   */
  auto connections(std::invocable<const std::map<std::string, std::vector<std::string>>&> auto&& handler) -> void {
    sdbusplus::asio::getProperty<std::string>(
        *connection_, ipc_ruler_service_name, ipc_ruler_object_path, ipc_ruler_interface_name, "Connections",
        [handler](const boost::system::error_code& error, const std::string& response) {
          if (error) {
            return;
          }
          auto slots = glz::read_json<std::map<std::string, std::vector<std::string>>>(response);
          if (slots) {
            handler(slots.value());
          }
        });
  }

  /**
   * Send a request over dbus to connect a slot to a signal.
   * On error handler will be called back with a non empty std::error_code&
   * @param slot_name the name of the slot to be connected
   * @param signal_name the name of the signal to be connected
   * @param handler a method that receives an error in case there is one
   * @note If the slot is already connected to a signal, it will be disconnected and connected to the new one
   */
  auto connect(const std::string& slot_name,
               const std::string& signal_name,
               std::invocable<const std::error_code&> auto&& handler) -> void {
    connection_->async_method_call(std::forward<decltype(handler)>(handler), ipc_ruler_service_name, ipc_ruler_object_path,
                                   ipc_ruler_interface_name, "Connect", slot_name, signal_name);
  }

  /**
   * Send a request over dbus to disconnect a slot from its signal
   * @tparam message_handler Type of the callback handler, this should be auto detected
   * @param slot_name the name of the slot to be disconnected
   * @param handler a method that receives an error in case there is one
   */
  template <typename message_handler>
  auto disconnect(const std::string& slot_name, message_handler&& handler) -> void {
    connection_->async_method_call(std::forward<decltype(handler)>(handler), ipc_ruler_service_name, ipc_ruler_object_path,
                                   ipc_ruler_interface_name, "disconnect", slot_name);
  }

  /**
   * Register a callback function that gets "pinged" each time there is a connection change
   * @param connection_change_callback a function like object on each connection change it gets called with the slot and
   * signal that changed.
   * @note There can only be a single callback registered per slot_name, if you register a new one. The old callback will be
   * overwritten.
   */
  auto register_connection_change_callback(std::string_view slot_name,
                                           const std::function<void(std::string_view const)>& connection_change_callback)
      -> void {
    slot_callbacks_.emplace(std::string(slot_name), connection_change_callback);
  }

private:
  auto match_callback(sdbusplus::message_t& msg) -> void {
    auto container = msg.unpack<std::tuple<std::string, std::string>>();
    std::string const slot_name = std::get<0>(container);
    std::string const signal_name = std::get<1>(container);
    auto iterator = slot_callbacks_.find(slot_name);
    if (iterator != slot_callbacks_.end()) {
      std::invoke(iterator->second, signal_name);
    }
  }

  std::unique_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::bus::match::match> match_;
  std::unordered_map<std::string, std::function<void(std::string_view const)>> slot_callbacks_;
};

struct ipc_manager_client_mock {
  auto register_connection_change_callback(std::string_view slot_name,
                                           const std::function<void(std::string_view const)>& connection_change_callback)
      -> void {
    slot_callbacks.emplace(std::string(slot_name), connection_change_callback);
  }

  std::unordered_map<std::string, std::function<void(std::string_view const)>> slot_callbacks;

  auto register_slot(const std::string_view name,
                     std::string_view description,
                     type_e type,
                     std::invocable<const std::error_code&> auto&& handler) -> void {
    slots.emplace_back(slot{ .name = std::string(name),
                             .type = type,
                             .created_by = "",
                             .created_at = std::chrono::system_clock::now(),
                             .last_registered = std::chrono::system_clock::now(),
                             .last_modified = std::chrono::system_clock::now(),
                             .modified_by = "",
                             .connected_to = "",
                             .description = std::string(description) });
    handler(std::error_code());
  }

  auto register_signal(const std::string_view name,
                       std::string_view description,
                       type_e type,
                       std::invocable<const std::error_code&> auto&& handler) -> void {
    signals.emplace_back(signal{ .name = std::string(name),
                                 .type = type,
                                 .created_by = "",
                                 .created_at = std::chrono::system_clock::now(),
                                 .last_registered = std::chrono::system_clock::now(),
                                 .description = std::string(description) });
    handler(std::error_code());
  }

  auto connect(const std::string& slot_name,
               const std::string& signal_name,
               std::invocable<const std::error_code&> auto&& handler) -> void {
    for (auto& slot : slots) {
      if (slot.name == slot_name) {
        slot.connected_to = signal_name;
        auto iterator = slot_callbacks.find(slot_name);
        if (iterator != slot_callbacks.end()) {
          std::invoke(iterator->second, signal_name);
        }
        std::error_code no_err{};
        handler(no_err);
        return;
      }
    }
    throw std::runtime_error("Signal not found in mocking list signal_name: " + signal_name + " slot_name: " + slot_name);
  }

  std::vector<slot> slots;
  std::vector<signal> signals;
};

}  // namespace tfc::ipc_ruler
