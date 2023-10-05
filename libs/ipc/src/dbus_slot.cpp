#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <fmt/format.h>

#include <tfc/ipc/details/dbus_slot.hpp>

namespace tfc::ipc::details {

static constexpr std::string_view value_property{ "Value" };

dbus_slot::dbus_slot(asio::io_context& ctx) : conn_{ std::make_shared<sdbusplus::asio::connection>(ctx) } {}
dbus_slot::dbus_slot(std::shared_ptr<sdbusplus::asio::connection> conn) : conn_{ conn } {}
void dbus_slot::initialize(std::string_view slot_name) {
  conn_->request_name(fmt::format("{}._slot_", slot_name).c_str());
}
asio::io_context& dbus_slot::io_context() const noexcept {
  return conn_->get_io_context();
}
std::shared_ptr<sdbusplus::asio::connection> dbus_slot::connection() const noexcept {
  return conn_;
}

}  // namespace tfc::ipc::details
