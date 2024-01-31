#include <chrono>
#include <functional>
#include <vector>

#include <async_mqtt/buffer.hpp>
#include <async_mqtt/packet/v5_publish.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <run.hpp>
#include <config/bridge.hpp>

namespace asio = boost::asio;

auto main(int argc, char* argv[]) -> int {
  auto program_description{ tfc::base::default_description() };

  tfc::base::init(argc, argv, program_description);

  asio::io_context io_ctx{};

  tfc::mqtt::run<tfc::mqtt::config::bridge_mock, tfc::mqtt::client_semi_normal, tfc::ipc_ruler::ipc_manager_client_mock&>

  tfc::mqtt::run<tfc::confman::config<tfc::mqtt::config::bridge>> running{ io_ctx };

  co_spawn(io_ctx, running.start(), asio::detached);

  io_ctx.run();

  return 0;
}
