/** \file
 * \brief Example code for running context_t
 *
 * Usage: context_example --iface IFACE --id <some_id> --stdout --log-level <log_level>
 * IFNAME is the NIC interface name, e.g. 'eth0'
 *
 */

#include <ranges>

#include <boost/asio/signal_set.hpp>

#include "tfc/ec.hpp"
#include "tfc/progbase.hpp"

import argparse;

auto main(int argc, char* argv[]) -> int {
  auto prog{ tfc::base::default_parser() };
  std::string iface;
  prog.add_argument("--iface").action([&iface](const std::string& val) { iface = val; }).help("Adapter name").required();
  tfc::base::init(argc, argv, prog);

  boost::asio::io_context io_ctx;
  tfc::ec::context_t ctx(io_ctx, iface);

  ctx.async_start();

  io_ctx.run();

  return 0;
}
