// ipc-ruler.cpp - Dbus API service maintaining a list of signals/slots and which signal
// is connected to which slot

#include <iostream>
#include <functional>
#include <string_view>
#include <unordered_map>

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

using std::string_literals::operator""s;

// D-Bus constants

// service name
static auto dbus_service_name = "com.skaginn3x.ipc_ruler"s;
// object path
static auto dbus_object_path = "/com/skaginn3x/ipc_ruler"s;

using user = std::tuple<std::string, int>;

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
  ipc_manager() : logger_("ipc_manager") {}
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
    if (slots_.find(slot_name) == slots_.end() || signals_.find(signal_name) == signals_.end()){
      throw std::runtime_error("Signal or slot does not exist");
    }
    if (slots_.at(slot_name).type != signals_.at(signal_name).type){
      throw std::runtime_error("Signal and slot types dont match");;
    }

    return true;
  }
  auto disconnect(const std::string& slot_name) -> bool {
    logger_.trace("disconnect called, slot: {}", slot_name);
    if (slots_.find(slot_name) == slots_.end()){
      throw std::runtime_error("Slot does not exist");
    }
    return true;
  }

private:
  std::unordered_map<std::string, signal> signals_;
  std::unordered_map<std::string, slot> slots_;
  tfc::logger::logger logger_;
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;
  auto connection = std::make_shared<sdbusplus::asio::connection>(ctx);

  connection->request_name(dbus_service_name.data());
  auto server = sdbusplus::asio::object_server(connection);

  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_iface =
      server.add_interface(dbus_object_path.data(), "com.skaginn3x.manager");

  ipc_manager m;

  // dbus_iface->register_method("register_signal", std::bind(&ipc_manager::register_signal, &m, std::placeholders::_1,
  // std::placeholders::_2));
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

  // dbus_iface->register_method("get_all_signals", std::bind_front(&ipc_manager::get_all_signals, m));

  dbus_iface->register_property("some_prop", 10, sdbusplus::asio::PropertyPermission::readWrite);
  dbus_iface->register_signal<std::tuple<std::string, std::string>>("connection_change");



  dbus_iface->initialize();

  // ctx.run_for(std::chrono::seconds(10));
  for(int i = 0; i < 100; i++){
    ctx.run_for(std::chrono::seconds(10));
  }
}
