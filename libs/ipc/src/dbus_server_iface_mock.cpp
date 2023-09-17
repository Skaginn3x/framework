#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/ipc/details/dbus_server_iface_mock.hpp>

#include <boost/asio/io_context.hpp>
#include <sdbusplus/message.hpp>

namespace tfc::ipc_ruler {

ipc_manager_client_mock::ipc_manager_client_mock(asio::io_context&) {}

void ipc_manager_client_mock::register_connection_change_callback(
    std::string_view slot_name,
    const std::function<void(const std::string_view)>& connection_change_callback) {
  slot_callbacks.emplace(std::string(slot_name), connection_change_callback);
}
void ipc_manager_client_mock::register_slot(std::string_view name, std::string_view description, ipc::details::type_e type) {
  slots_.emplace_back(slot{ .name = std::string(name),
                            .type = type,
                            .created_by = "",
                            .created_at = std::chrono::system_clock::now(),
                            .last_registered = std::chrono::system_clock::now(),
                            .last_modified = std::chrono::system_clock::now(),
                            .modified_by = "",
                            .connected_to = "",
                            .description = std::string(description) });
}
void ipc_manager_client_mock::register_signal(std::string_view name,
                                              std::string_view description,
                                              ipc::details::type_e type) {
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
}
auto ipc_manager_client_mock::register_properties_change_callback(std::function<void(sdbusplus::message_t&)> const& property_callback) -> std::unique_ptr<sdbusplus::bus::match::match> {
  callbacks_.emplace_back(property_callback);
  return nullptr;
}

}  // namespace tfc::ipc_ruler
