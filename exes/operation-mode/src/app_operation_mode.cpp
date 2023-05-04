#include "app_operation_mode.hpp"

#include <string>

#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/operation_mode/common.hpp>

namespace tfc {

// todo move to dbus_utils
static auto open_sd_bus() {
  sd_bus* bus = nullptr;
  if (sd_bus_open(&bus) < 0) {
    throw std::runtime_error(std::string("Unable to open sd-bus, error: ") + strerror(errno));
  }
  return bus;
}

app_operation_mode::app_operation_mode(boost::asio::io_context& ctx)
    : dbus_{ std::make_shared<sdbusplus::asio::connection>(ctx, open_sd_bus()) },
      dbus_object_server_{
        std::unique_ptr<sdbusplus::asio::object_server, std::function<void(sdbusplus::asio::object_server*)>>(
            new sdbusplus::asio::object_server(dbus_),
            [](sdbusplus::asio::object_server* ptr) { delete ptr; })
      },
      dbus_interface_{ dbus_object_server_->add_interface(operation::dbus::path.data(), operation::dbus::name.data()) } {
  dbus_interface_->register_signal<>()
}

}  // namespace tfc
