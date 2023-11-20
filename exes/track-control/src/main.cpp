#include <cstdlib>

#include <boost/asio.hpp>

#include <tfc/progbase.hpp>
#include <tfc/track/ctrl.hpp>

namespace asio = boost::asio;

auto main(int argc, char const* const* const argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::track::ctrl const ctrl{ ctx };

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
