#include <tfc/operation_mode.hpp>

import asio;
import tfc.base;
import sdbus;

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::operation::interface mode {
    ctx
  };

  mode.on_leave(tfc::operation::mode_e::stopped, [](tfc::operation::mode_e new_mode, tfc::operation::mode_e old_mode) {
    fmt::print("Leaving {} and going to: {}", enum_name(old_mode), enum_name(new_mode));
  });

  ctx.run();

  return EXIT_SUCCESS;
}
