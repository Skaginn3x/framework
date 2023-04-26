// ipc-ruler.cpp - Dbus API service maintaining a list of signals/slots and which signal
// is connected to which slot

#include <iostream>
#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>

#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"

using std::string_view_literals::operator""sv;

// D-Bus constants

// service name
static constexpr auto dbus_service_name = "com.skaginn3x.ipc_ruler"sv;
// object path
static constexpr auto dbus_object_path = "/com/skaginn3x/ipc_ruler"sv;

struct user {
  std::string name;
  int age;
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  boost::asio::io_context ctx;
  auto connection = std::make_shared<sdbusplus::asio::connection>(ctx);

  connection->request_name(dbus_service_name.data());
  auto server = sdbusplus::asio::object_server(connection);
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_iface =
      server.add_interface(dbus_object_path.data(), "com.skaginn3x.some_interface");

  dbus_iface->register_property("count", 0, sdbusplus::asio::PropertyPermission::readWrite);

  std::vector<std::string> myStringVec = { "some", "test", "data" };
  std::vector<int> myIntVec = { 1, 2, 3 };

  dbus_iface->register_method("TestFunction",
                              [](int number, std::string name) { std::cout << number << name << std::endl; });
  dbus_iface->register_property("vector_of_string", myStringVec, sdbusplus::asio::PropertyPermission::readWrite);
  dbus_iface->register_property("vector_of_int", myIntVec);

  dbus_iface->initialize();
  ctx.run();
  std::cout << "Complete" << std::endl;
}
