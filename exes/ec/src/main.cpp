/** \file
 * \brief Example code for running context_t
 *
 * Usage: context_example --iface IFACE --id <some_id> --stdout --log-level <log_level>
 * IFNAME is the NIC interface name, e.g. 'eth0'
 *
 */

#include <ranges>
#include <sched.h>

#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include "tfc/ec.hpp"
#include "tfc/progbase.hpp"

auto main(int argc, char* argv[]) -> int {

  sched_param scheduling{ .sched_priority = sched_get_priority_max(SCHED_FIFO) };
  if (scheduling.sched_priority < 0) {
    throw std::runtime_error{ fmt::format("Unable to get scheduling priority, error: {}", strerror(errno))};
  }
  auto const me{ 0 };
  if (sched_setscheduler(me, SCHED_FIFO, &scheduling) < 0) {
    throw std::runtime_error{ fmt::format("Unable to set scheduling priority, error: {}", strerror(errno))};
  }

  auto prog_desc{ tfc::base::default_description() };
  std::string iface;
  prog_desc.add_options()("iface,i", boost::program_options::value<std::string>(&iface)->required(), "Adapter name");
  tfc::base::init(argc, argv, prog_desc);

  boost::asio::io_context io_ctx;
  tfc::ec::context_t ctx(io_ctx, iface);

  ctx.async_start();

  io_ctx.run();

  return 0;
}
