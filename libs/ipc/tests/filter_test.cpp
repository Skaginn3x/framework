#include <chrono>
#include <string>

#include <tfc/ipc.hpp>
#include <tfc/ipc/details/filter.hpp>

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

  "filter edge timer"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<type_e::timer, bool> timer_test{};
          auto return_value = co_await timer_test.async_process(true, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);

          co_return;  //
        },
        asio::detached);
    ctx.run_one();
  };
  return 0;
}
