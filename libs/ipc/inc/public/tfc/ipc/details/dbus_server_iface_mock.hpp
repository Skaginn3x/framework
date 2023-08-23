#pragma once

// ipc-manager-client-mock
// mocks the behavior of ipc-manager-client

#include "dbus_server_iface.hpp"

namespace tfc::ipc_ruler {

struct ipc_manager_client_mock {
  ipc_manager_client_mock() {}

  ipc_manager_client_mock(boost::asio::io_context&) {}

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
    slots_.emplace_back(slot{ .name = std::string(name),
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
    signals_.emplace_back(signal{ .name = std::string(name),
                                  .type = type,
                                  .created_by = "",
                                  .created_at = std::chrono::system_clock::now(),
                                  .last_registered = std::chrono::system_clock::now(),
                                  .description = std::string(description) });

    sdbusplus::message_t dbus_message = sdbusplus::message_t{};
    for (auto& callback : callbacks_) {
      callback(dbus_message);
    }

    handler(std::error_code());
  }

  auto connect(const std::string& slot_name,
               const std::string& signal_name,
               std::invocable<const std::error_code&> auto&& handler) -> void {
    for (auto& slot : slots_) {
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

  auto slots(std::invocable<const std::vector<slot>&> auto&& handler) -> void { handler(slots_); }

  auto signals(std::invocable<const std::vector<signal>&> auto&& handler) -> void { handler(signals_); }

  template <typename callback>
  auto register_properties_change_callback(callback&& property_callback) -> std::unique_ptr<sdbusplus::bus::match::match> {
    callbacks_.emplace_back(std::forward<callback>(property_callback));
    return nullptr;
  }

  std::vector<slot> slots_;
  std::vector<signal> signals_;
  std::vector<std::function<void(sdbusplus::message_t&)>> callbacks_ = {};
};

}  // namespace tfc::ipc_ruler
