#pragma once

#include <memory>

namespace boost::asio{
class io_context;
}
namespace sdbusplus::asio {
class connection;
}


class connection_test {
public:
  explicit connection_test(boost::asio::io_context& ctx);
  ~connection_test();
  sdbusplus::asio::connection* dbus_;
};
