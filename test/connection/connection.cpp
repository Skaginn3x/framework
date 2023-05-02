#include "connection.hpp"
#include <systemd/sd-bus.h>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>

connection_test::connection_test(boost::asio::io_context& ctx): descriptor_{ nullptr} {
  int const err{ sd_bus_default(&bus_) };
  if (err < 0) {
    throw "hello world";
  }
  int const file_descriptor{ sd_bus_get_fd(bus_) };
  descriptor_ = std::make_shared<boost::asio::posix::stream_descriptor>(ctx, file_descriptor);
  descriptor_->async_wait(boost::asio::posix::descriptor_base::wait_read, [](std::error_code const&){
    printf("hello world\n");
  });
}
connection_test::~connection_test() {
  sd_bus_flush(bus_);
  sd_bus_close(bus_);
  sd_bus_unref(bus_);
  bus_ = nullptr;
}
