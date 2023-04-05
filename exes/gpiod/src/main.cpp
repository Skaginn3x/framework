#include <cstdlib>
#include <string>

#include <boost/program_options.hpp>
#include <boost/asio.hpp>

#include <tfc/progbase.hpp>

#include "gpio.h"

namespace bpo = boost::program_options;
namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };
  std::string device{};
  desc.add_options()("device,d", bpo::value<std::string>(&device)->default_value("/dev/gpiochip0"), "GPIO character device.");
  tfc::base::init(argc, argv, desc);

  asio::io_context ctx{};

  [[maybe_unused]] auto instance{ tfc::gpio{ ctx, device } };

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}