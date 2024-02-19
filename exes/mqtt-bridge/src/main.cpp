#include <boost/asio.hpp>

#include <tfc/progbase.hpp>

#include <run.hpp>

namespace asio = boost::asio;

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  asio::io_context io_ctx{};

  tfc::mqtt::run<

tfc::confman::config<tfc::mqtt::config::bridge>,
 tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::confman::config<tfc::mqtt::config::bridge>>,
  tfc::ipc_ruler::ipc_manager_client
    > running{ io_ctx };

  co_spawn(io_ctx, running.start(), asio::detached);

  io_ctx.run();

  return 0;
}

// template class spark_plug_interface<confman::config<config::bridge>, client_n>;
// template class spark_plug_interface<config::bridge_mock, client_mock>;
// template class spark_plug_interface<config::bridge_mock, client_semi_normal>;
