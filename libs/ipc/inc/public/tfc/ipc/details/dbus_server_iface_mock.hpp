#pragma once

// ipc-manager-client-mock
// mocks the behavior of ipc-manager-client

#include <chrono>
#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <tfc/utils/asio_fwd.hpp>

#include <tfc/ipc/details/dbus_iface_structs.hpp>
#include <tfc/ipc/enums.hpp>

namespace tfc::ipc_ruler {

namespace asio = boost::asio;

struct ipc_manager_client_mock {
  ipc_manager_client_mock() = default;

  ipc_manager_client_mock(asio::io_context&);

  void register_connection_change_callback(std::string_view slot_name,
                                           const std::function<void(std::string_view const)>& connection_change_callback);

  void register_slot(std::string_view name, std::string_view description, ipc::details::type_e type);
  void register_slot(std::string_view name,
                     std::string_view description,
                     ipc::details::type_e type,
                     std::invocable<const std::error_code&> auto&& handler) {
    register_slot(name, description, type);
    handler(std::error_code());
  }

  void register_signal(std::string_view name, std::string_view description, ipc::details::type_e type);
  auto register_signal(std::string_view name,
                       std::string_view description,
                       ipc::details::type_e type,
                       std::invocable<const std::error_code&> auto&& handler) -> void {
    register_signal(name, description, type);
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

  auto register_properties_change_callback(std::function<void(sdbusplus::message_t&)> const&)
      -> std::unique_ptr<sdbusplus::bus::match::match>;

  std::vector<slot> slots_;
  std::vector<signal> signals_;
  std::vector<std::function<void(sdbusplus::message_t&)>> callbacks_ = {};
  std::unordered_map<std::string, std::function<void(std::string_view const)>> slot_callbacks;
};

}  // namespace tfc::ipc_ruler
