#include <chrono>
#include <string>

#include <tfc/ipc.hpp>
#include <tfc/ipc/details/filter.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/testing/asio_clock.hpp>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

using tfc::ipc::filter::filter;
using tfc::ipc::filter::filter_e;

using ut::operator""_test;
using ut::expect;
using ut::operator>>;
using ut::fatal;
using ut::operator|;

struct timer_test {
  asio::io_context ctx{};
  bool finished{ false };
  filter<filter_e::timer, bool, tfc::testing::clock> timer{};
  void spawn(auto&& callback) const {}
  ~timer_test() { ut::expect(finished); }
};

auto main(int, char**) -> int {
  "happy path filter edge timer"_test = [](bool test_value) {
    timer_test test{};
    asio::co_spawn(
        test.ctx,
        [&test, test_value]() -> asio::awaitable<void> {
          auto return_value = co_await test.timer.async_process(bool{ test_value }, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value() == test_value);
          test.finished = true;
          co_return;  //
        },
        asio::detached);
    test.ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    test.ctx.run_one_for(std::chrono::seconds{ 1 });  // timer event 1
  } | std::vector{ true, false };

  "filter edge delayed"_test = [](bool test_value) {
    timer_test test{};
    test.timer.time_on = std::chrono::milliseconds{ 1 };
    test.timer.time_off = std::chrono::milliseconds{ 1 };
    asio::co_spawn(
        test.ctx,
        [&test, test_value]() -> asio::awaitable<void> {
          test.timer.async_process(bool{ test_value },
                                   asio::bind_executor(test.ctx.get_executor(), [&test, test_value](auto&& return_value) {
                                     expect(return_value.has_value() >> fatal);
                                     expect(return_value.value() == test_value);
                                     test.finished = true;
                                   }));
          co_return;
        },
        asio::detached);
    test.ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    test.ctx.run_one_for(std::chrono::seconds{ 1 });  // async process (async_compose)
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 1 });
    test.ctx.run_one_for(std::chrono::seconds{ 1 });  // poll timer once more, `now` should be at this moment
  } | std::vector{ true, false };

  "edge back to previous state within the delayed time"_test = []() {
    timer_test test{};
    test.timer.time_on = std::chrono::milliseconds{ 1 };
    test.timer.time_off = std::chrono::milliseconds{ 1 };
    asio::co_spawn(
        test.ctx,
        [&test, test_value = true]() -> asio::awaitable<void> {
          test.timer.async_process(bool{ test_value }, [](auto&& return_value) {
            expect(!return_value.has_value() >> fatal);
            // the following async process call should cancel the timer so the error here is cancelled
            expect(return_value.error() == std::errc::operation_canceled);
          });
          test.timer.async_process(!test_value, [](auto&&) {
            // this callback should never be called since we are back to the previous state
            expect(false >> fatal);
          });
          test.finished = true;
          co_return;
        },
        asio::detached);
    test.ctx.run_one_for(std::chrono::seconds{ 1 });  // co_spawn
    test.ctx.run_one_for(std::chrono::seconds{ 1 });  // async_process
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 1 });
    test.ctx.run_for(std::chrono::milliseconds{ 50 });  // try polling for this given time
  };

  "move filter during processing happy path filter edge timer"_test = [](bool test_value) {
    bool finished{ false };
    asio::io_context ctx{};
    auto const start_process{ [&finished, &ctx, test_value]() -> filter<filter_e::timer, bool, tfc::testing::clock> {
      filter<filter_e::timer, bool, tfc::testing::clock> timer_test{};
      timer_test.time_on = std::chrono::milliseconds{ 1 };
      timer_test.time_off = std::chrono::milliseconds{ 1 };
      asio::co_spawn(
          ctx,
          [&finished, &timer_test, test_value, &ctx]() -> asio::awaitable<void> {
            timer_test.async_process(bool{ test_value },
                                     asio::bind_executor(ctx.get_executor(), [&finished, test_value](auto&& return_value) {
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
  } | std::vector{ true, false };

  "confman integrated timer filter"_test = [](bool test_value) {
    bool finished{ false };
    asio::io_context ctx{};
    tfc::confman::stub_config<filter<filter_e::timer, bool, tfc::testing::clock>> config{ ctx, "my_key" };
    config.make_change()->time_on = std::chrono::milliseconds{ 10 };
    config.make_change()->time_off = std::chrono::milliseconds{ 10 };
    asio::co_spawn(
        ctx,
        [&finished, &config, test_value, &ctx]() -> asio::awaitable<void> {
          config->async_process(bool{ test_value },
                                asio::bind_executor(ctx.get_executor(), [&finished, test_value](auto&& return_value) {
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
    ctx.run_one_for(std::chrono::milliseconds{ 30 });  // poll timer once more, but nothing happens
    // down to business, 1 millisecond has elapsed since the timer was made with 10 millisec wait let's change the value
    expect(!finished);

    auto generic_config = glz::read_json<glz::json_t>(config.string());
    expect(generic_config.has_value() >> fatal);
    generic_config->at("time_on") = 1;  // reduce time to only 1 millisecond
    generic_config->at("time_off") = 1;
    expect(!config.from_string(glz::write_json(generic_config)) >> fatal);
    expect(config->time_on == std::chrono::milliseconds{ 1 });
    expect(config->time_off == std::chrono::milliseconds{ 1 });

    // IMPORTANT: increase time by 9 since the previous declared time needs to elapse until we get called again
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 9 });
    ctx.run_one_for(std::chrono::seconds{ 1 });  // poll timer once more, `now` should be at this moment
    // now the callback should have been called
    expect(finished);
  } | std::vector{ true, false };

  "filter invert"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<filter_e::invert, bool> const invert_test{};
          auto return_value = co_await invert_test.async_process(true, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(!return_value.value());
          return_value = co_await invert_test.async_process(false, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value());
          co_return;  //
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });
  };

  "filter offset"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<filter_e::offset, std::int64_t> offset_test{ .offset = 2 };
          auto return_value = co_await offset_test.async_process(40, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value() == 42);
          offset_test.offset = -2;
          return_value = co_await offset_test.async_process(44, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value() = 42);
          co_return;  //
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });
  };

  "filter multiply"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<filter_e::multiply, std::double_t> multiply_test{ .multiply = 2.5 };
          auto return_value = co_await multiply_test.async_process(40, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value() > 99 && return_value.value() < 101);
          multiply_test.multiply = -2.5;
          return_value = co_await multiply_test.async_process(40, asio::use_awaitable);
          expect(return_value.has_value() >> fatal);
          expect(return_value.value() < -99 && return_value.value() > -101);
          co_return;  //
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });
  };

  "mass filter out"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<filter_e::filter_out, tfc::ipc::details::mass_t> filter_out_test{ .filter_out = 100 * mp_units::si::gram };
          auto return_value = co_await filter_out_test.async_process(100 * mp_units::si::gram, asio::use_awaitable);
          expect(!return_value.has_value());
          expect(return_value.value().value() != 100 * mp_units::si::gram);
          return_value = co_await filter_out_test.async_process(40 * mp_units::si::gram, asio::use_awaitable);
          expect(return_value.has_value());
          expect(return_value.value().value() == 40 * mp_units::si::gram);
          co_return;
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });
  };

  "mass filter offset"_test = []() {
    asio::io_context ctx{};
    asio::co_spawn(
        ctx,
        []() -> asio::awaitable<void> {
          filter<filter_e::offset, tfc::ipc::details::mass_t> filter_offset_test{ .filter_offset =
                                                                                      100 * mp_units::si::gram };
          auto return_value = co_await filter_offset_test.async_process(100 * mp_units::si::gram, asio::use_awaitable);
          expect(return_value.has_value());
          expect(return_value.value().value() == 200 * mp_units::si::gram);
          return_value = co_await filter_offset_test.async_process(40 * mp_units::si::gram, asio::use_awaitable);
          expect(return_value.has_value());
          expect(return_value.value().value() == 140 * mp_units::si::gram);
          co_return;
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });
  };

  return 0;
}
