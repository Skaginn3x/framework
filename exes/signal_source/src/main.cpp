#include <cstdlib>
#include <string>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <map>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

PRAGMA_CLANG_WARNING_PUSH_OFF(-Wdate-time)
#include <pcg_extras.hpp>
PRAGMA_CLANG_WARNING_POP
#include <random>
#include <pcg_random.hpp>

namespace bpo = boost::program_options;
namespace asio = boost::asio;

using std::chrono::milliseconds;
using std::chrono_literals::operator""ms;

inline auto blink(boost::asio::io_context& ctx, milliseconds const& period, tfc::ipc_ruler::ipc_manager_client& client)
    -> asio::awaitable<void> {
  asio::steady_timer timer{ ctx, period };
  double frequency = 1.0 / (static_cast<double>(period.count()) * 2.0 / 1000.0);
  tfc::ipc::bool_signal signal{ ctx, client, fmt::format("square_wave_{}", period),
                                fmt::format("Boolean square wave {}, {:.3f}Hz", period, frequency) };
  bool state{ false };
  for (;;) {
    timer.expires_after(period);
    co_await timer.async_wait(asio::use_awaitable);
    state = !state;
    co_await signal.async_send(state, asio::use_awaitable);
  }
}

inline auto random_int(boost::asio::io_context& ctx, std::int64_t const& min, std::int64_t const& max, tfc::ipc_ruler::ipc_manager_client& client)
-> asio::awaitable<void> {
  std::chrono::milliseconds period = 100ms;
  asio::steady_timer timer{ ctx, period };
  double frequency = 1.0 / (static_cast<double>(period.count()) * 2.0 / 1000.0);
  tfc::ipc::int_signal signal{ ctx, client, fmt::format("uniform_random_int_{}_{}_{}", (min < 0 ? fmt::format("neg_{}", std::abs(min)) : std::to_string(min)), max, period),
                                fmt::format("Random int, changing every {}, {:.3f}Hz", period, frequency) };

  auto new_state = [min, max]() -> std::int64_t {
    static pcg64 random_engine(std::random_device{}());
    std::uniform_int_distribution<std::int64_t> dist{ min, max };
    return dist(random_engine);
  };
  std::int64_t state{};
  for (;;) {
    timer.expires_after(period);
    co_await timer.async_wait(asio::use_awaitable);
    state = new_state();
    co_await signal.async_send(state, asio::use_awaitable);
  }
}

inline auto random_double(boost::asio::io_context& ctx, std::int64_t const& min, std::int64_t const& max, tfc::ipc_ruler::ipc_manager_client& client)
-> asio::awaitable<void> {
  std::chrono::milliseconds period = 100ms;
  asio::steady_timer timer{ ctx, period };
  double frequency = 1.0 / (static_cast<double>(period.count()) * 2.0 / 1000.0);
  tfc::ipc::double_signal signal{ ctx, client, fmt::format("uniform_random_double_{}_{}_{}", (min < 0 ? fmt::format("neg_{}", std::abs(min)) : std::to_string(min)), max, period),
                               fmt::format("Random double, changing every {}, {:.3f}Hz", period, frequency) };

  auto new_state = [min, max]() -> double {
    static pcg64 random_engine(std::random_device{}());
    std::uniform_real_distribution<double> dist{ static_cast<double>(min), static_cast<double>(max) };
    return dist(random_engine);
  };
  double state{};
  for (;;) {
    timer.expires_after(period);
    co_await timer.async_wait(asio::use_awaitable);
    state = new_state();
    co_await signal.async_send(state, asio::use_awaitable);
  }
}

using mp_units::quantity;
using mp_units::si::gram;
using signal_t = tfc::ipc::details::type_mass::value_t::value_type;
inline auto random_weight(boost::asio::io_context& ctx, signal_t const min, signal_t const max, tfc::ipc_ruler::ipc_manager_client& client)
-> asio::awaitable<void> {
  std::chrono::milliseconds period = 100ms;
  asio::steady_timer timer{ ctx, period };
  double frequency = 1.0 / (static_cast<double>(period.count()) * 2.0 / 1000.0);
  tfc::ipc::mass_signal signal{ ctx, client, fmt::format("uniform_random_mass_{}_{}_{}", 0, 100, period),
                                  fmt::format("Random mass, changing every {}, {:.3f}Hz", period, frequency) };

  auto new_state = []() -> signal_t {
    static pcg64 random_engine(std::random_device{}());
    std::uniform_real_distribution<std::int64_t> dist{-100, 100}; //TODO: Make this right
    return dist(random_engine) * gram;
  };
  signal_t state{};
  for (;;) {
    timer.expires_after(period);
    co_await timer.async_wait(asio::use_awaitable);
    state = new_state();
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

  co_spawn(ctx, random_int(ctx, -100, 100, client), asio::detached);
  co_spawn(ctx, random_int(ctx, 0, 100, client), asio::detached);
  // +1 because of abs for the negative sign
  co_spawn(ctx, random_int(ctx, std::numeric_limits<std::int64_t>::min()+1, std::numeric_limits<std::int64_t>::max(), client), asio::detached);

  co_spawn(ctx, random_double(ctx, -100.0, 100.0, client), asio::detached);
  co_spawn(ctx, random_double(ctx, 0.0, 100.0, client), asio::detached);
  // +1 for the abs for the negative sign
  co_spawn(ctx, random_double(ctx, -10'000'000, 10'000'000, client), asio::detached);



  co_spawn(ctx, random_weight(ctx, -100 * gram, 100 * gram, client), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
