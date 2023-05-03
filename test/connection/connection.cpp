#include "connection.hpp"
#include <boost/asio.hpp>


connection_test::connection_test(boost::asio::io_context& ctx): dbus_{ new sdbusplus::asio::connection(ctx) } {
}
connection_test::~connection_test() {
  delete dbus_;
}
