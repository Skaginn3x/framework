#include <boost/asio.hpp>
#include "app_operation_mode.hpp"

import tfc.base;
import asio;

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  [[maybe_unused]] auto const app{ tfc::app_operation_mode<>(ctx) };

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
