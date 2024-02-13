#include <boost/asio.hpp>
#include <tfc/progbase.hpp>
#include "app_operation_mode.hpp"

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  [[maybe_unused]] auto const app{ tfc::app_operation_mode<>(ctx) };

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  tfc::base::run(ctx);

  return EXIT_SUCCESS;
}
