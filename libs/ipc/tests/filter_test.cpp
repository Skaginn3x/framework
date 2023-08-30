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

  using tfc::ipc::filter::filter;
  using tfc::ipc::filter::type_e;

  "filter invert"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<type_e::invert, bool> invert_test{};
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

  "happy path filter edge timer"_test = []() {
    asio::io_context ctx{};
    bool finished{ false };
    asio::co_spawn(
        ctx,
        [&finished]() -> asio::awaitable<void> {
          auto constexpr mini_test{ [](bool test_value) -> asio::awaitable<void> {
            filter<type_e::timer, bool, tfc::testing::clock> timer_test{};
            auto return_value = co_await timer_test.async_process(test_value, asio::use_awaitable);
            expect(return_value.has_value() >> fatal);
            expect(return_value.value() == test_value);
            co_return;
          } };
          co_await mini_test(true);
          co_await mini_test(false);
          finished = true;
          co_return;  //
        }, asio::detached);
    ctx.run_one_for(std::chrono::seconds{1}); // co_spawn
    ctx.run_one_for(std::chrono::seconds{1}); // timer event 1
    ctx.run_one_for(std::chrono::seconds{1}); // timer event 2
    expect(finished);
  };

  "filter edge timer cancels"_test = []() {
    asio::io_context ctx{};
    bool finished{ false };
    asio::co_spawn(
        ctx,
        [&finished]() -> asio::awaitable<void> {
          filter<type_e::timer, bool, tfc::testing::clock> timer_test{.time_on = std::chrono::milliseconds{ 1 }};
          timer_test.async_process(test_value, [&finished](auto&& return_value){
            expect(return_value.has_value() >> fatal);
            expect(return_value.value() == test_value);
            finished = true;
          });
          co_return;  //
        }, asio::detached);
    ctx.run_one_for(std::chrono::seconds{1}); // co_spawn
    ctx.run_one_for(std::chrono::seconds{1}); // timer event 1
    ctx.run_one_for(std::chrono::seconds{1}); // timer event 2
    expect(finished);
  };

  return 0;
}
