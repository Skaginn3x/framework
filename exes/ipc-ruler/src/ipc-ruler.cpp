// ipc-ruler.cpp - Dbus API service maintaining a list of signals/slots and which signal
// is connected to which slot

#include <iostream>
#include <functional>
#include <concepts>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"
#include "tfc/dbus_util.hpp"

using std::string_literals::operator""s;

// D-Bus constants

// service name
static auto dbus_service_name = "com.skaginn3x.ipc_ruler"s;
// object path
static auto dbus_object_path = "/com/skaginn3x/ipc_ruler"s;

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
  ipc_manager(std::function<void (std::string, std::string)> on_connect_cb) : logger_("ipc_manager"), on_connect_cb{on_connect_cb} {}
  auto register_signal(std::string name, int type) -> bool {
    logger_.trace("register_signal called name: {}, type: {}", name, type);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto* timestamp_now = std::ctime(&now);
    if (signals_.find(name) != signals_.end()) {
      signals_.at(name).last_registered = timestamp_now;
      return true;
    }
    signals_.emplace(name, signal{ .name = name,
                                   .type = type,
                                   .created_by = "omar",
                                   .created_at = timestamp_now,
                                   .last_registered = timestamp_now });
    return true;
  }
  auto register_slot(const std::string& name, int type) -> bool {
    logger_.trace("register_slot called name: {}, type: {}", name, type);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto* timestamp_now = std::ctime(&now);

    auto never = std::chrono::system_clock::to_time_t(std::chrono::time_point<std::chrono::system_clock>{});
    auto* timestamp_never = std::ctime(&never);
    if (slots_.find(name) != slots_.end()) {
      slots_.at(name).last_registered = timestamp_now;
      return true;
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
    return true;
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

  auto connect(const std::string& slot_name, const std::string& signal_name) -> bool {
    logger_.trace("connect called, slot: {}, signal: {}", slot_name, signal_name);
    if (slots_.find(slot_name) == slots_.end()){
      //throw sdbusplus::exception::InvalidEnumString();
      throw dbus_error(fmt::format("Slot ({}) does not exist", slot_name));
    }
    if (signals_.find(signal_name) == signals_.end()){
      throw dbus_error(fmt::format("Signal ({}) does not exist", signal_name));
    }
    if (slots_.at(slot_name).type != signals_.at(signal_name).type){
      throw dbus_error("Signal and slot types dont match");
    }

    slots_.at(slot_name).connected_to = signal_name;
    on_connect_cb(slot_name, signal_name);
    // send a notification
    return true;
  }
  auto disconnect(const std::string& slot_name) -> bool {
    logger_.trace("disconnect called, slot: {}", slot_name);
    if (slots_.find(slot_name) == slots_.end()){
      throw std::runtime_error("Slot does not exist");
    }
    slots_.at(slot_name).connected_to = "";
    on_connect_cb(slot_name, "");
    return true;
  }

private:
  std::unordered_map<std::string, signal> signals_;
  std::unordered_map<std::string, slot> slots_;
  tfc::logger::logger logger_;
  std::function<void (std::string, std::string)> on_connect_cb;
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;
  auto connection = std::make_shared<sdbusplus::asio::connection>(ctx);

  connection->request_name(dbus_service_name.data());
  auto server = sdbusplus::asio::object_server(connection);

  // Example of how to introspect
  // busctl introspect --user com.skaginn3x.ipc_ruler /com/skaginn3x/ipc_ruler
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_iface =
      server.add_interface(dbus_object_path.data(), "com.skaginn3x.manager");

  ipc_manager m([&dbus_iface](std::string slot_name, std::string signal_name){
    auto message = dbus_iface->new_signal("connection_change");
    message.append(std::tuple<std::string, std::string>(slot_name, signal_name));
    message.signal_send();
  });

  dbus_iface->register_method("register_signal",
                              [&m](const std::string& name, int type) { return m.register_signal(name, type); });
  dbus_iface->register_method("register_slot",
                              [&m](const std::string& name, int type) { return m.register_slot(name, type); });

  dbus_iface->register_method("get_all_signals", [&m]() { return m.get_all_signals(); });
  dbus_iface->register_method("get_all_slots", [&m]() { return m.get_all_slots(); });

  dbus_iface->register_method("connect", [&m](const std::string& slot_name, const std::string& signal_name) {
    return m.connect(slot_name, signal_name);
  });

  dbus_iface->register_method("disconnect", [&m](const std::string& slot_name) { return m.disconnect(slot_name); });

  dbus_iface->register_signal<std::tuple<std::string, std::string>>("connection_change");


  dbus_iface->initialize();

  ctx.run();
}
