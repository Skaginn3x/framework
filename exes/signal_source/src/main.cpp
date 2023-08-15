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
using std::chrono_literals::operator""ms;

inline auto blink(boost::asio::io_context& ctx, milliseconds const& period, tfc::ipc_ruler::ipc_manager_client& client)
    -> asio::awaitable<void> {
  asio::steady_timer timer{ ctx, period };
  tfc::ipc::bool_signal signal{ ctx, client, fmt::format("blinker_{}", period), "Period blinker" };
  bool state{ false };
  for (;;) {
    timer.expires_after(period);
    co_await timer.async_wait(asio::use_awaitable);
    state = !state;
    co_await signal.async_send(state, asio::use_awaitable);
  }
}

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  tfc::ipc_ruler::ipc_manager_client client{ ctx };

  for (const auto& blink_duration :
       { 100ms, 200ms, 300ms, 400ms, 500ms, 750ms, 1000ms, 1500ms, 2000ms, 3000ms, 4000ms, 5000ms, 7500ms, 10000ms }) {
    co_spawn(ctx, blink(ctx, blink_duration, client), asio::detached);
  }

  ctx.run();

  return EXIT_SUCCESS;
}
