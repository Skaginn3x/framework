#include <tfc/ipc/details/dbus_client_iface_mock.hpp>

#include <boost/asio/io_context.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/asio/connection.hpp>

namespace tfc::ipc_ruler {

ipc_manager_client_mock::ipc_manager_client_mock(std::shared_ptr<sdbusplus::asio::connection> conn)
    : conn_{ std::move(conn) } {}
ipc_manager_client_mock::ipc_manager_client_mock(asio::io_context& ctx)
    : ipc_manager_client_mock{ std::make_shared<sdbusplus::asio::connection>(ctx) } {}

void ipc_manager_client_mock::register_connection_change_callback(
    std::string_view slot_name,
    const std::function<void(const std::string_view)>& connection_change_callback) {
  slot_callbacks.emplace(std::string(slot_name), connection_change_callback);
}
void ipc_manager_client_mock::register_slot_retry(std::string_view name,
                                                  std::string_view description,
                                                  ipc::details::type_e type) {
  auto now{ std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()) };
  slots_.emplace_back(slot{ .name = std::string(name),
                            .type = type,
                            .created_by = "",
                            .created_at = now,
                            .last_registered = now,
                            .last_modified = now,
                            .modified_by = "",
                            .connected_to = "",
                            .description = std::string(description) });
}
void ipc_manager_client_mock::register_signal_retry(std::string_view name,
                                                    std::string_view description,
                                                    ipc::details::type_e type) {
  auto now{ std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()) };
  signals_.emplace_back(signal{ .name = std::string(name),
                                .type = type,
                                .created_by = "",
                                .created_at = now,
                                .last_registered = now,
                                .description = std::string(description) });

  sdbusplus::message_t dbus_message = sdbusplus::message_t{};
  for (auto& callback : callbacks_) {
    callback(dbus_message);
  }
}
auto ipc_manager_client_mock::register_properties_change_callback(
    std::function<void(sdbusplus::message_t&)> const& property_callback) -> std::unique_ptr<sdbusplus::bus::match::match> {
  callbacks_.emplace_back(property_callback);
  return nullptr;
}

}  // namespace tfc::ipc_ruler
