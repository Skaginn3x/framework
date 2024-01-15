/** \file
 * \brief Example code for running context_t
 *
 * Usage: context_example --iface IFACE --id <some_id> --stdout --log-level <log_level>
 * IFNAME is the NIC interface name, e.g. 'eth0'
 *
 */

#include <sys/resource.h>
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

  rlimit limit{};
  auto const res{ getrlimit(RLIMIT_NOFILE, &limit) };
  if (res != 0) {
    fmt::println(stderr, "Failed to getrlimit: {}", std::strerror(errno));
    return 1;
  }
  fmt::println("File descriptor soft limit: {}", limit.rlim_cur);
  fmt::println("File descriptor hard limit: {}", limit.rlim_max);

  ctx.async_start();

  io_ctx.run();

  return 0;
}
