#pragma once

#include <expected>

#include <tfc/confman/detail/common.hpp>
#include <tfc/progbase.hpp>
#include <tfc/rpc.hpp>
#include <tfc/stx/concepts.hpp>

#include <fmt/format.h>
#include <azmq/socket.hpp>
#include <boost/asio.hpp>

namespace asio = boost::asio;

namespace tfc::confman::detail {

using server_t = tfc::rpc::server<glz::rpc::server<glz::rpc::server_method_t<"alive", alive, alive_result>>>;

class config_rpc_server {
public:
  explicit config_rpc_server(asio::io_context& ctx)
      : server_{ ctx, rpc_socket }, notifications_{ ctx } {
  }

private:

  server_t server_;
  azmq::pub_socket notifications_;
};

}  // namespace tfc::confman::detail

