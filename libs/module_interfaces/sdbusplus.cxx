module;
#include <chrono>
#include <boost/asio.hpp>
export module sdbus;

import std;

extern "C++" {
#include <sdbusplus/sdbus.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/message.hpp>
}


export namespace sdbusplus {
  using sdbusplus::asio::connection;
  using sdbusplus::asio::object_server;
  using sdbusplus::asio::dbus_interface;

  //using sdbusplus::bus::match;
  using sdbusplus::message::message;
}
