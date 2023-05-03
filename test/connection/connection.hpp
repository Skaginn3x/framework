#pragma once

#include <memory>
#include <sdbusplus/asio/connection.hpp>



class connection_test {
public:
  explicit connection_test(boost::asio::io_context& ctx);
  ~connection_test();
  sdbusplus::asio::connection* dbus_;
};
