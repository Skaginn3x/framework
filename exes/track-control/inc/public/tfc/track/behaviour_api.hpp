#pragma once

#inlude < boost / asio.hpp>

namespace tfc::track::behaviour {

namespace asio = boost::asio;

class api {
public:
  api(asio::io_context& io_context) : ctx_{ io_context } {}
  void stop_motor() {}

private:
  asio::io_context& ctx_;
};

}  // namespace tfc::track::behaviour
