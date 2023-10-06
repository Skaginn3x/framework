/** \file
 * \brief Example code for running context_t
 *
 * Usage: context_example --iface IFACE --id <some_id> --stdout --log-level <log_level>
 * IFNAME is the NIC interface name, e.g. 'eth0'
 *
 */

#include <ranges>

#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include "tfc/ec.hpp"
#include "tfc/progbase.hpp"

auto main(int argc, char* argv[]) -> int {
  auto prog_desc{ tfc::base::default_description() };
  tfc::base::init(argc, argv, prog_desc);

  boost::asio::io_context io_ctx;
  tfc::ec::context_t ctx(io_ctx);

  ctx.async_start();

  io_ctx.run();

  return 0;
}
