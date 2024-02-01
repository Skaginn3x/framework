#include <boost/asio.hpp>

#include <tfc/progbase.hpp>

#include <run.hpp>

namespace asio = boost::asio;

auto main(int argc, char* argv[]) -> int {
  tfc::base::init(argc, argv);

  asio::io_context io_ctx{};

  tfc::mqtt::run<> running{ io_ctx };

  co_spawn(io_ctx, running.start(), asio::detached);

  io_ctx.run();

  return 0;
}
