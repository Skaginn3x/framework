#pragma once

#include <memory>

namespace boost::asio
{
class io_context;
class any_io_executor;
}

namespace boost::asio::posix
{
template<typename executor_t>
class basic_stream_descriptor;
using stream_descriptor = basic_stream_descriptor<any_io_executor>;
}
using sd_bus = struct sd_bus;


class connection_test {
public:
  connection_test(boost::asio::io_context& ctx);
  ~connection_test();
  sd_bus* bus_{ nullptr };
  std::shared_ptr<boost::asio::posix::stream_descriptor> descriptor_;
};
