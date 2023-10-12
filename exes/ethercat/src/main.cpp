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
  tfc::base::init(argc, argv);

  boost::asio::io_context io_ctx;
  tfc::ec::context_t ctx(io_ctx);

  while (!ctx.init()){
    // While there is no interface defined just wait.
    // We need this program running to accept interface changes
    // but we cannot do anything until something is working.
    io_ctx.run_for(std::chrono::milliseconds (1500));
  }
  ctx.async_start();

  io_ctx.run();

  return 0;
}
