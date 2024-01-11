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

#include <sqlite_modern_cpp.h>

namespace tfc::ipc_ruler {
using tfc::ipc::details::type_e;

using dbus_error = tfc::dbus::exception::runtime;

/**
 * A class exposing methods for managing signals and slots
 */
class ipc_manager {
public:
  using slot_name = std::string_view;
  using signal_name = std::string_view;

  explicit ipc_manager(bool in_memory = false)
      : db_(in_memory ? ":memory:" : base::make_config_file_name("ipc-ruler", "db")) {
    db_ << R"(
          CREATE TABLE IF NOT EXISTS signals(
              name TEXT,
              type INT,
              created_by TEXT,
              created_at LONG INTEGER,
              time_point_t LONG INTEGER,
              last_registered LONG INTEGER,
              description TEXT);
             )";
    db_ << R"(
          CREATE TABLE IF NOT EXISTS slots(
              name TEXT,
              type INT,
              created_by TEXT,
              created_at LONG INTEGER,
              last_registered LONG INTEGER,
              last_modified INTEGER,
              modified_by TEXT,
              connected_to TEXT,
              time_point_t LONG INTEGER,
              description TEXT);
             )";
  }

  auto set_callback(std::function<void(slot_name, signal_name)> on_connect_cb) -> void {
    on_connect_cb_ = std::move(on_connect_cb);
  }

  auto register_signal(const std::string_view name, const std::string_view description, type_e type) -> void {
    logger_.trace("register_signal called name: {}, type: {}", name, enum_name(type));
    auto timestamp_now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

    try {
      // Check if signal exists
      int count = 0;
      db_ << "select count(*) from signals where name = ?;" << std::string(name) >> count;
      if (count != 0) {
        // update the signal
        db_ << fmt::format("UPDATE signals SET last_registered = {}, description = '{}', type = {}  WHERE name = '{}';",
                           timestamp_now.time_since_epoch().count(), description, static_cast<std::uint8_t>(type), name);
      } else {
        // Insert the signal
        db_ << fmt::format(
            "INSERT INTO signals (name, type, created_at, last_registered, description) VALUES ('{}',{},{},{},'{}');", name,
            static_cast<std::uint8_t>(type), timestamp_now.time_since_epoch().count(),
            timestamp_now.time_since_epoch().count(), description);
      }
    } catch (const std::exception& e) {
      logger_.error(e.what());
    }
  }

  auto register_slot(const std::string_view name, const std::string_view description, type_e type) -> void {
    logger_.trace("register_slot called name: {}, type: {}", name, enum_name(type));
    auto timestamp_now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto timestamp_never = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>{};

    // Call the connected callback to get the slot connected to its signal if it has one.
    try {
      // Check if signal exists

      std::string connected_to = "";
      bool found = false;
      db_ << "select connected_to from slots where name = ?;" << std::string(name) >>
          [&connected_to, &found](const std::string& result) {
            connected_to = result;
            found = true;
          };
      if (found) {
        // update the signal
        db_ << fmt::format("UPDATE slots SET last_registered = {}, description = '{}', type = {}  WHERE name = '{}';",
                           timestamp_now.time_since_epoch().count(), description, static_cast<std::uint8_t>(type), name);
      } else {
        // Insert the signal
        db_ << fmt::format(
            "INSERT INTO slots (name, type, created_at, last_registered, last_modified, description) VALUES "
            "('{}',{},{},{},{},'{}');",
            name, static_cast<std::uint8_t>(type), timestamp_now.time_since_epoch().count(),
            timestamp_now.time_since_epoch().count(), timestamp_never.time_since_epoch().count(), description);
      }

      on_connect_cb_(name, connected_to);
    } catch (const std::exception& e) {
      logger_.error(e.what());
    }
  }

  auto get_all_signals() -> std::vector<signal> {
    logger_.trace("get_all_signals called");
    std::vector<signal> ret;
    db_ << "select name, type, last_registered, description, created_at, created_by from signals" >>
        [&ret](const std::string& name, const int type, const std::uint64_t last_registered, const std::string& description,
               const std::uint64_t created_at, const std::string& created_by) {
          using std::chrono::milliseconds;
          const auto last_reg = time_point_t(milliseconds(last_registered));
          const auto cre_at = time_point_t(milliseconds(created_at));
          ret.emplace_back(signal{ name, static_cast<type_e>(type), created_by, cre_at, last_reg, description });
        };
    return ret;
  }

  auto get_all_slots() -> std::vector<slot> {
    logger_.trace("get_all_slots called");
    std::vector<slot> ret;
    db_ << "select name, type, last_registered, description, created_at, created_by, last_modified, modified_by, "
           "connected_to from slots" >>
        [&ret](const std::string& name, const int type, const std::uint64_t last_registered, const std::string& description,
               const std::uint64_t created_at, const std::string& created_by, const std::uint64_t last_modified,
               const std::string& modified_by, const std::string& connected_to) {
          using std::chrono::milliseconds;
          const auto last_reg = time_point_t(milliseconds(last_registered));
          const auto cre_at = time_point_t(milliseconds(created_at));
          const auto last_mod = time_point_t(milliseconds(last_modified));
          ret.emplace_back(slot{ name, static_cast<type_e>(type), created_by, cre_at, last_reg, last_mod, modified_by,
                                 connected_to, description });
        };
    return ret;
  }

  auto get_all_connections() -> std::map<std::string, std::vector<std::string>> {
    std::map<std::string, std::vector<std::string>> connections;
    db_ << "SELECT signals.name, slots.name FROM signals JOIN slots on signals.name = slots.connected_to;" >>
        [&connections](const std::string& signal_name, const std::string& slot_name) {
          connections[signal_name].emplace_back(slot_name);
        };
    return connections;
  }

  auto connect(const std::string_view slot_name, const std::string_view signal_name) -> void {
    try {
      logger_.trace("connect called, slot: {}, signal: {}", slot_name, signal_name);
      int slot_count = 0;
      db_ << "select count(*) from slots where name = ?;" << std::string(slot_name) >> slot_count;
      if (slot_count == 0) {
        std::string const err_msg = fmt::format("Slot ({}) does not exist", slot_name);
        logger_.warn(err_msg);
        throw dbus_error(err_msg);
      }
      int signal_count = 0;
      db_ << "select count(*) from signals where name = ?;" << std::string(signal_name) >> signal_count;
      if (signal_count == 0) {
        std::string const err_msg = fmt::format("Signal ({}) does not exist", signal_name);
        logger_.warn(err_msg);
        throw dbus_error(err_msg);
      }

      int signal_type = 0;
      db_ << fmt::format("select type from signals where name = '{}' LIMIT 1;", slot_name) >>
          [&signal_type](const int result) { signal_type = result; };
      int slot_type = 0;
      db_ << fmt::format("select type from slots where name = ? LIMIT 1;", slot_name) >>
          [&slot_type](const int result) { slot_type = result; };
      if (signal_type != slot_type) {
        std::string const err_msg = "Signal and slot types dont match";
        logger_.warn(err_msg);
        throw dbus_error(err_msg);
      }

      db_ << fmt::format("UPDATE slots SET connected_to = '{}' WHERE name = '{}';", signal_name, slot_name);
      on_connect_cb_(slot_name, signal_name);
    } catch (const std::exception& e) {
      logger_.warn(e.what());
    }
  }

  auto disconnect(const std::string_view slot_name) -> void {
    logger_.trace("disconnect called, slot: {}", slot_name);
    try {
      int slot_count = 0;
      db_ << "select count(*) from slots where name = ?;" << std::string(slot_name) >> slot_count;
      if (slot_count == 0) {
        throw std::runtime_error("Slot does not exist");
      }
      db_ << fmt::format("UPDATE slots SET connected_to = '' WHERE name = '{}';", slot_name);
      on_connect_cb_(slot_name, "");
    } catch (const std::exception& e) {
      logger_.warn(e.what());
    }
  }

private:
  logger::logger logger_{ "ipc-manager" };
  sqlite::database db_;
  std::function<void(std::string_view, std::string_view)> on_connect_cb_;
};

class ipc_manager_server {
public:
  explicit ipc_manager_server(boost::asio::io_context& ctx, std::unique_ptr<ipc_manager>&& ipc_manager)
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
  std::unique_ptr<ipc_manager> ipc_manager_;
};
}  // namespace tfc::ipc_ruler
