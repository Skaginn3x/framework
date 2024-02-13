#include <cstdlib>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/sml.hpp>

#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;

namespace sml = boost::sml;
using sml::operator""_s;
using sml::operator""_e;

using std::chrono::milliseconds;
using std::chrono_literals::operator""s;
using std::chrono_literals::operator""ms;
using std::chrono::steady_clock;

static constexpr auto long_press_upper_bound = 7s;

// Events
struct high {};
struct low {};
struct timeout {};

struct my_deps {
  my_deps(asio::io_context& ctx_para, tfc::logger::logger& logger_para, tfc::ipc_ruler::ipc_manager_client& client)
      : ctx{ ctx_para }, logger{ logger_para }, double_signal{ ctx, client, "double_tap", "Button double tap" },
        long_signal{ ctx, client, "long_press", "Button long press" }, touch_signal{ ctx, client, "touch", "Button touch" },
        initial_time(steady_clock::now()) {}
  asio::io_context& ctx;
  tfc::logger::logger& logger;
  tfc::ipc::bool_signal double_signal;
  tfc::ipc::bool_signal long_signal;
  tfc::ipc::bool_signal touch_signal;
  std::chrono::time_point<steady_clock, std::chrono::nanoseconds> initial_time;
};

// guards
constexpr auto initial_time_set = [](my_deps& deps) { deps.initial_time = steady_clock::now(); };
constexpr auto long_press = [](my_deps& deps) {
  auto delta = steady_clock::now() - deps.initial_time;
  return delta > 1000ms && delta < long_press_upper_bound;
};

constexpr auto short_press = [](my_deps& deps) {
  auto delta = steady_clock::now() - deps.initial_time;
  return delta < 1000ms;
};

constexpr auto too_long_press = [](my_deps& deps) {
  auto delta = steady_clock::now() - deps.initial_time;
  return delta >= long_press_upper_bound;
};

// actions
constexpr auto start_timeout = [](const auto&, auto& state_machine, auto& deps, const auto& subs) {
  // Create a timer from a shared_ptr
  auto& deps_v = deps.value;
  auto timer = std::make_shared<asio::steady_timer>(deps_v.ctx);
  auto left = 500ms;
  timer->expires_after(left);
  // Copy the timer into the callback to make another shared_ptr
  // reference
  timer->async_wait([&, timer_copy = timer](const std::error_code& err) {
    if (err) {
      return;
    }
    state_machine.process_event(timeout{}, deps, subs);
  });
};

// Actions
constexpr auto send_signal = [](asio::io_context& ctx, tfc::ipc::bool_signal& signal, tfc::logger::logger& logger) {
  signal.async_send(true, [&](const std::error_code& err, size_t) {
    if (err) {
      logger.warn("Failed to send, {}", err.message());
    }
  });

  // Create a timer from a shared_ptr
  auto timer = std::make_shared<asio::steady_timer>(ctx);
  timer->expires_after(100ms);

  // Copy the timer into the callback to make another shared_ptr
  // reference
  timer->async_wait([&, timer_copy = timer](const std::error_code& timer_err) {
    if (timer_err) {
      return;
    }
    signal.async_send(false, [&](const std::error_code& err, size_t) {
      if (err) {
        logger.warn("Failed to write low: {}", err.message());
      }
    });
  });
};

constexpr auto send_long_press = [](my_deps& deps) { return send_signal(deps.ctx, deps.long_signal, deps.logger); };

constexpr auto send_touch = [](my_deps& deps) { return send_signal(deps.ctx, deps.touch_signal, deps.logger); };

constexpr auto send_double_tap = [](my_deps& deps) { return send_signal(deps.ctx, deps.double_signal, deps.logger); };

/// A state machine for turning boolean signals into button events.
struct button_state {
  auto operator()() {
    return sml::make_transition_table(
        *"start"_s + sml::event<high> / initial_time_set = "choose"_s,
        "choose"_s + sml::event<low>[long_press] / send_long_press = "start"_s,
        "choose"_s + sml::event<low>[short_press] / start_timeout = "wait"_s,
        "choose"_s + sml::event<low>[too_long_press] = "start"_s,
        "wait"_s + sml::event<high>[short_press] = "send_double"_s,  // Wait for low also on double tap
        "wait"_s + sml::event<high>[long_press] = "start"_s, "wait"_s + sml::event<timeout> / send_touch = "start"_s,
        "send_double"_s + sml::event<low> / send_double_tap = "start"_s);
  }
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};
  tfc::ipc_ruler::ipc_manager_client client{ ctx };

  tfc::logger::logger logger("button");

  auto deps = my_deps{ ctx, logger, client };

  sml::sm<button_state> sm(deps);

  tfc::ipc::bool_signal raw_signal{ ctx, client, "raw_value", "Raw button value" };
  tfc::ipc::bool_slot input_slot{ ctx, client, "input", "Button input", [&](bool value) {
                                   logger.trace("Value: {}", value);
                                   raw_signal.async_send(value, [&](const std::error_code& err, size_t) {
                                     if (err) {
                                       logger.warn("Failed to send raw_value, {}", err.message());
                                     }
                                   });
                                   if (value) {
                                     sm.process_event(sml::event<high>());
                                   } else {
                                     sm.process_event(sml::event<low>());
                                   }
                                 } };

  ctx.run();

  return EXIT_SUCCESS;
}
