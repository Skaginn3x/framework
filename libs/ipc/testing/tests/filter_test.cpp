#include <chrono>
#include <string>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/filter.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/testing/clock.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

using std::chrono::operator""ms;
using std::chrono::operator""s;

using tfc::ipc::filter::filter;
using tfc::ipc::filter::filter_e;
using timer_filter = filter<filter_e::timer, bool, asio::basic_waitable_timer<tfc::testing::clock, tfc::testing::wait_traits>>;

using ut::operator""_test;
using ut::expect;
using ut::operator>>;
using ut::fatal;
using ut::operator|;

struct timer_test {
  asio::io_context ctx{};
  bool finished{ false };
  timer_filter timer{};
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
    test.ctx.run_one_for(1ms);  // co_spawn
    test.ctx.run_one_for(1ms);  // timer event 1
  } | std::vector{ true, false };

  "filter edge delayed"_test = [](bool test_value) {
    timer_test test{};
    test.timer.time_on = 42s;
    test.timer.time_off = 42s;
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
    test.ctx.run_one_for(1ms);  // co_spawn
    test.ctx.run_one_for(1ms);  // async process (async_compose)
    ut::expect(!test.finished);
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + 42s);
    test.ctx.run_one_for(42s);  // poll timer once more, `now` should be at this moment
  } | std::vector{ true, false };

  "edge back to previous state within the delayed time"_test = []() {
    timer_test test{};
    test.timer.time_on = 42s;
    test.timer.time_off = 42s;
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
    test.ctx.run_one_for(1ms);  // co_spawn
    test.ctx.run_one_for(1ms);  // async_process
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + 42s);
    test.ctx.run_for( 1ms );  // try polling for this given time
  };

  "move filter during processing happy path filter edge timer"_test = [](bool test_value) {
    bool finished{ false };
    asio::io_context ctx{};
    auto const start_process{ [&finished, &ctx, test_value]() -> timer_filter {
      timer_filter timer_test{};
      timer_test.time_on = 42s;
      timer_test.time_off = 42s;
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
      ctx.run_one_for(1ms);  // co_spawn
      ctx.run_one_for(1ms);  // async process (async_compose)
      return timer_test;
    } };

    auto call_move_constructor{ start_process() };
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + 42s);
    ctx.run_one_for(1ms);  // poll timer once more, `now` should be at this moment
    expect(finished);
  } | std::vector{ true, false };

  "confman integrated timer filter"_test = [](bool test_value) {
    bool finished{ false };
    asio::io_context ctx{};
    auto dbus{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) };
    tfc::confman::stub_config<filter<filter_e::timer, bool, asio::basic_waitable_timer<tfc::testing::clock, tfc::testing::wait_traits>>> config{ dbus, "my_key" };
    config.make_change()->time_on = 42s ;
    config.make_change()->time_off = 42s ;
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
    ctx.run_one_for(1ms);  // co_spawn
    ctx.run_one_for(1ms);  // async process (async_compose)
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + 1s);
    ctx.run_one_for(1ms );  // poll timer once more, but nothing happens
    // down to business, 1 second has elapsed since the timer was made with 42 second wait, let's change the value
    expect(!finished);

    auto generic_config = glz::read_json<glz::json_t>(config.string());
    expect(generic_config.has_value() >> fatal);
    generic_config->at("time_on") = 1;  // reduce time to only 1 millisecond
    generic_config->at("time_off") = 1;
    expect(!config.from_string(glz::write_json(generic_config)) >> fatal);
    expect(config->time_on == 1ms);
    expect(config->time_off == 1ms);

    // IMPORTANT: increase time by 41 second since the previous declared time needs to elapse until we get called again
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + 41s);
    // Not sure why we need 3 polls
    ctx.run_one_for(1ms);  // poll timer once more, `now` should be at this moment
    ctx.run_one_for(1ms);  // poll timer once more, `now` should be at this moment
    ctx.run_one_for(1ms);  // poll timer once more, `now` should be at this moment
    // now the callback should have been called
    // fixed in https://github.com/Skaginn3x/framework/pull/545
    // expect(finished);
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
    ctx.run_one_for(1ms);
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
    ctx.run_one_for(1ms);
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
    ctx.run_one_for(1ms);
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
    ctx.run_one_for(1ms);
  };

  [[maybe_unused]] ut::suite<"filter invert config changes"> invert_config = [] {
    struct invert_config_test {
      asio::io_context ctx{};
      std::shared_ptr<sdbusplus::asio::connection> connection{ std::make_shared<sdbusplus::asio::connection>(ctx) };
      std::function<void(bool)> callback{ [](bool) {} };
      tfc::ipc::filter::filters<bool, decltype(callback), tfc::confman::stub_config<tfc::ipc::filter::observable_config_t<bool>>> filters{ connection, "foo", callback };
    };
    "add invert doesn't call owner when no value has been received"_test = [] {
      invert_config_test test{ .callback = [](bool) {
        expect(false);
      } };
      {
        auto config = test.filters.config()->value();
        config.emplace_back(filter<filter_e::invert, bool>{});
        test.filters.config().make_change().value() = config;
      }
    };
    "add invert calls owner with inverted value"_test = [] {
      std::size_t call_count{ 0 };
      invert_config_test test{ .callback = [&call_count](bool value) {
        if (call_count == 0) {
          expect(!value); // initialize value
        }
        else {
          expect(value);
        }
        call_count++;
      } };
      test.filters(false); // initial value
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 1);
      {
        auto config = test.filters.config()->value();
        config.emplace_back(filter<filter_e::invert, bool>{});
        test.filters.config().make_change().value() = config;
      }
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 2);
    };
    "adding 2 inverts does NOT call owner with value"_test = [] {
      std::size_t call_count{ 0 };
      invert_config_test test{ .callback = [&call_count](bool value) {
        if (call_count == 0) {
          expect(!value); // initialize value
        }
        else {
          expect(value);
        }
        call_count++;
      } };
      test.filters(false); // initial value
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 1);
      {
        auto config = test.filters.config()->value();
        config.emplace_back(filter<filter_e::invert, bool>{});
        config.emplace_back(filter<filter_e::invert, bool>{});
        test.filters.config().make_change().value() = config;
      }
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 1);
    };
    "adding 3 inverts DOES call owner with inverted value"_test = [] {
      std::size_t call_count{ 0 };
      invert_config_test test{ .callback = [&call_count](bool value) {
        if (call_count == 0) {
          expect(!value); // initialize value
        }
        else {
          expect(value);
        }
        call_count++;
      } };
      test.filters(false); // initial value
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 1);
      {
        auto config = test.filters.config()->value();
        config.emplace_back(filter<filter_e::invert, bool>{});
        config.emplace_back(filter<filter_e::invert, bool>{});
        config.emplace_back(filter<filter_e::invert, bool>{});
        test.filters.config().make_change().value() = config;
      }
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 2);
    };
    "remove invert DOES call owner with inverted value"_test = [] {
      // Duplicate of adding 2 inverts
      std::size_t call_count{ 0 };
      invert_config_test test{ .callback = [&call_count](bool value) {
        if (call_count == 0) {
          expect(!value); // initialize value
        }
        else {
          expect(value);
        }
        call_count++;
      } };
      test.filters(false); // initial value
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 1);
      {
        auto config = test.filters.config()->value();
        config.emplace_back(filter<filter_e::invert, bool>{});
        config.emplace_back(filter<filter_e::invert, bool>{});
        test.filters.config().make_change().value() = config;
      }
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 1);
      // End of duplicate
      // now we have 2 inverts,
      // remove one and the owner should be called with the inverted value
      {
        auto config = test.filters.config()->value();
        config.pop_back();
        test.filters.config().make_change().value() = config;
      }
      test.ctx.run_one_for(1ms);
      test.ctx.run_one_for(1ms);
      expect(call_count == 2);
    };

  };

  return 0;
}
