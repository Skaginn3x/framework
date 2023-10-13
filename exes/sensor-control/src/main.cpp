#include <cstdlib>

#include <boost/asio.hpp>

#include <tfc/progbase.hpp>
#include "sensor_control.hpp"

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::sensor_control<tfc::ipc::signal> const ctrl{ ctx };

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
