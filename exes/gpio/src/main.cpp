#include "gpio.hpp"

import tfc.base;
import std;
import argparse;
import asio;

namespace bpo = boost::program_options;
namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_parser() };
  std::string device{};
  desc.add_argument("-d", "--device").action([&device](const std::string& val){
                                       device = val;
                                     }).default_value(std::string("/dev/gpiochip0")).help("GPIO character device.");
  tfc::base::init(argc, argv, desc);

  asio::io_context ctx{};

  [[maybe_unused]] auto instance{ tfc::gpio{ ctx, device } };

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
