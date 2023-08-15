#include <cstdlib>
#include <string>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace bpo = boost::program_options;
namespace asio = boost::asio;

using std::chrono::milliseconds;

struct blinker {
  blinker(boost::asio::io_context& ctx, milliseconds const& period, tfc::ipc_ruler::ipc_manager_client& mgr)
      : timer{ ctx, period }, period{ period }, logger(fmt::format("blinker_{}", period)),
        signal(ctx, mgr, fmt::format("blinker_{}", period), "Period blinker") {}

  void start() {
    timer.expires_after(period);
    timer.async_wait([this](auto const& ec) {
      if (ec) {
        return;
      }
      state = !state;
      signal.async_send(state, [&](const std::error_code& err, size_t) {
        if (err) {
          logger.error("Failed to send signal: {}", err.message());
        }
      });
      logger.trace("Blinking ({}): {}", period, state);
      start();
    });
  }

  asio::steady_timer timer;
  milliseconds period;
  tfc::logger::logger logger;
  tfc::ipc::bool_signal signal;
  bool state = false;
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  tfc::ipc_ruler::ipc_manager_client client{ ctx };

  std::vector<blinker> blinkers_instances;

  auto blinkers = { milliseconds{ 100 },  milliseconds{ 200 },  milliseconds{ 300 },  milliseconds{ 400 },
                    milliseconds{ 500 },  milliseconds{ 750 },  milliseconds{ 1000 }, milliseconds{ 1500 },
                    milliseconds{ 2000 }, milliseconds{ 3000 }, milliseconds{ 4000 }, milliseconds{ 5000 },
                    milliseconds{ 7500 }, milliseconds{ 10000 } };

  for (auto& blink_duration : blinkers) {
    blinkers_instances.emplace_back(ctx, blink_duration, client);
  }

  for (auto& blink_instance : blinkers_instances) {
    blink_instance.start();
  }

  ctx.run();

  return EXIT_SUCCESS;
}
