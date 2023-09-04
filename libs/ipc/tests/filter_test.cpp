#include <chrono>
#include <string>

#include <tfc/ipc.hpp>
#include <tfc/ipc/details/filter.hpp>
#include <tfc/testing/asio_clock.hpp>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <boost/asio.hpp>
#include <boost/ut.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

auto main(int, char**) -> int {
  using ut::operator""_test;
  using ut::expect;
  using ut::operator>>;
  using ut::fatal;
  using ut::operator|;

  using tfc::ipc::filter::filter;
  using tfc::ipc::filter::type_e;

  "filter invert"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<type_e::invert, bool> const invert_test{};
          auto return_value = co_await invert_test.async_process(true, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(!return_value.value());
          return_value = co_await invert_test.async_process(false, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value());
          co_return;  //
        },
        asio::detached);
    ctx.run_one();
  };

  "happy path filter edge timer"_test = [](bool test_value) {
    asio::io_context ctx{};
    bool finished{ false };
    asio::co_spawn(
        ctx,
        [&finished, test_value]() -> asio::awaitable<void> {
          filter<type_e::timer, bool, tfc::testing::clock> const timer_test{};
          auto return_value = co_await timer_test.async_process(test_value, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value() == test_value);
          finished = true;
          co_return;  //
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    ctx.run_one_for(std::chrono::seconds{ 1 });  // timer event 1
    expect(finished);
  } | std::vector{ true, false };

  "filter edge delayed"_test = [](bool test_value) {
    bool finished{ false };
    asio::io_context ctx{};
    filter<type_e::timer, bool, tfc::testing::clock> timer_test{};
    timer_test.time_on = std::chrono::milliseconds{ 1 };
    timer_test.time_off = std::chrono::milliseconds{ 1 };
    asio::co_spawn(
        ctx,
        [&finished, &timer_test, test_value, &ctx]() -> asio::awaitable<void> {
          timer_test.async_process(test_value, asio::bind_executor(ctx.get_executor(), [&finished, test_value](auto&& return_value) {
            expect(return_value.has_value() >> fatal);
            expect(return_value.value() == test_value);
            finished = true;
          }));
          co_return;
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    ctx.run_one_for(std::chrono::seconds{ 1 });  // async process (async_compose)
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 1 });
    ctx.run_one_for(std::chrono::seconds{ 1 });  // poll timer once more, `now` should be at this moment
    expect(finished);
  } | std::vector{true, false};

  "edge back to previous state within the delayed time"_test = []() {
    bool finished{ false };
    asio::io_context ctx{};
    filter<type_e::timer, bool, tfc::testing::clock> timer_test{};
    timer_test.time_on = std::chrono::milliseconds{ 1 };
    timer_test.time_off = std::chrono::milliseconds{ 1 };
    asio::co_spawn(
        ctx,
        [&finished, &timer_test, test_value = true]() -> asio::awaitable<void> {
          timer_test.async_process(test_value, [](auto&& return_value) {
            expect(!return_value.has_value() >> fatal);
            // the following async process call should cancel the timer so the error here is cancelled
            expect(return_value.error() == std::errc::operation_canceled);
          });
          timer_test.async_process(!test_value, [](auto&&) {
            // this callback should never be called since we are back to the previous state
            expect(false >> fatal);
          });
          finished = true;
          co_return;
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    ctx.run_one_for(std::chrono::seconds{ 1 });  // async_process
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 1 });
    ctx.run_for(std::chrono::milliseconds{ 50 });  // try polling for this given time
    expect(finished);
  };

  "move filter during processing happy path filter edge timer"_test = [](bool test_value) {
    bool finished{ false };
    asio::io_context ctx{};
    auto const start_process{ [&finished, &ctx, test_value]() -> filter<type_e::timer, bool, tfc::testing::clock> {
      filter<type_e::timer, bool, tfc::testing::clock> timer_test{};
      timer_test.time_on = std::chrono::milliseconds{ 1 };
      timer_test.time_off = std::chrono::milliseconds{ 1 };
      asio::co_spawn(
          ctx,
          [&finished, &timer_test, test_value, &ctx]() -> asio::awaitable<void> {
            timer_test.async_process(test_value, asio::bind_executor(ctx.get_executor(), [&finished, test_value](auto&& return_value) {
              expect(return_value.has_value() >> fatal);
              expect(return_value.value() == test_value);
              finished = true;
            }));
            co_return;
          },
          asio::detached);
      ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
      ctx.run_one_for(std::chrono::seconds{ 1 });  // async process (async_compose)
      return timer_test;
    } };

    auto call_move_constructor{ start_process() };
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 1 });
    ctx.run_one_for(std::chrono::seconds{ 1 });  // poll timer once more, `now` should be at this moment
    expect(finished);
  } | std::vector{true, false};

  return 0;
}
