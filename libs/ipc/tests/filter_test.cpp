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

struct filter_test {};

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
        [&ctx]() -> asio::awaitable<void> {
          filter<type_e::invert, bool> invert_test{ ctx };
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
        [&finished, &ctx]() -> asio::awaitable<void> {
          auto constexpr mini_test{ [](bool test_value, asio::io_context& this_ctx) -> asio::awaitable<void> {
            filter<type_e::timer, bool, tfc::testing::clock> timer_test{ this_ctx };
            auto return_value = co_await timer_test.async_process(test_value, asio::use_awaitable);
            expect(return_value.has_value() >> fatal);
            expect(return_value.value() == test_value);
            co_return;
          } };
          co_await mini_test(true, ctx);
          co_await mini_test(false, ctx);
          finished = true;
          co_return;  //
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    ctx.run_one_for(std::chrono::seconds{ 1 });  // timer event 1
    ctx.run_one_for(std::chrono::seconds{ 1 });  // timer event 2
    expect(finished);
  };

  "filter edge timer delayed"_test = []() {
    bool finished{ false };
    asio::io_context ctx{};
    filter<type_e::timer, bool, tfc::testing::clock> timer_test{ ctx };
    //    asio::basic_waitable_timer<tfc::testing::clock> timer_test{ ctx };

    asio::co_spawn(
        ctx,
        [&finished, &timer_test]() -> asio::awaitable<void> {
          timer_test.time_on = std::chrono::milliseconds{ 1 };
          bool constexpr test_value{ true };
          timer_test.async_process(test_value, [&finished](auto&& return_value) {
            expect(return_value.has_value() >> fatal);
            expect(return_value.value() == test_value);
            finished = true;
          });
          //          timer_test.expires_after(std::chrono::milliseconds{ 1 });
          //          timer_test.async_wait([&finished](std::error_code const& err) {
          //            if (!err) {
          //              finished = true;
          //              return;
          //            }
          //            expect(false);
          //          });
          tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 1 });
          co_return;
        },
        asio::detached);
    //    ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    //    ctx.run_one_for(std::chrono::seconds{ 1 });  // timer event 1
    //    ctx.run_one_for(std::chrono::seconds{ 1 });  // timer event 1
    ctx.run_for(std::chrono::seconds{ 3 });
    expect(finished);
  };

  return 0;
}
