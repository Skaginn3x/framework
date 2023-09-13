#pragma once

#include <functional>
#include <memory>

#include <tfc/logger.hpp>
#include <tfc/operation_mode/common.hpp>

namespace boost::asio {
class io_context;
}  // namespace boost::asio

namespace sdbusplus::asio {
class connection;
class object_server;
class dbus_interface;
}  // namespace sdbusplus::asio

namespace tfc {

namespace operation {
class state_machine;
}

class app_operation_mode {
public:
  explicit app_operation_mode(boost::asio::io_context&);

private:
  std::shared_ptr<sdbusplus::asio::connection> dbus_;
  std::unique_ptr<sdbusplus::asio::object_server, std::function<void(sdbusplus::asio::object_server*)>> dbus_object_server_;
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  std::unique_ptr<operation::state_machine, std::function<void(operation::state_machine*)>> state_machine_;
  tfc::operation::mode_e mode_{ tfc::operation::mode_e::unknown };
  tfc::logger::logger logger_;
};

}  // namespace tfc
