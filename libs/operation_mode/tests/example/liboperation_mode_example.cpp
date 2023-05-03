
#include <boost/asio.hpp>
#include <tfc/progbase.hpp>
#include <tfc/operation_mode.hpp>

#include <sdbusplus/asio/connection.hpp>

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::operation::interface mode{ctx};

  mode.on_leave(tfc::operation::mode_e::stopped, [](tfc::operation::mode_e, tfc::operation::mode_e){});

  ctx.run();

  return EXIT_SUCCESS;
}
