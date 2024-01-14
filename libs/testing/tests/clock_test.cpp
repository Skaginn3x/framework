#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/testing/asio_clock.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

int main() {
  using ut::operator""_test;
  using ut::expect;

  "timer test"_test = [] {
    asio::io_context ctx{};
    asio::basic_waitable_timer<tfc::testing::clock, tfc::testing::wait_traits> timer{ ctx };
    timer.expires_after(std::chrono::milliseconds{ 30 });
    bool finished{ false };
    timer.async_wait([&finished](std::error_code const& err) {
      if (!err) {
        finished = true;
        return;
      }
      expect(false);
    });
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 30 });
    ctx.run_one_for(std::chrono::milliseconds{ 10 });
    expect(finished);
  };

  "more timer test"_test = [] {
    asio::io_context ctx{};
    asio::basic_waitable_timer<tfc::testing::clock, tfc::testing::wait_traits> timer{ ctx };
    timer.expires_after(std::chrono::milliseconds{ 30 });
    bool finished{ false };
    timer.async_wait([&finished](std::error_code const& err) {
      if (!err) {
        finished = true;
        return;
      }
      expect(false);
    });
    ctx.run_one_for(std::chrono::milliseconds{ 10 });
    expect(!finished);
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 10 });
    ctx.run_one_for(std::chrono::milliseconds{ 10 });
    expect(!finished);
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 20 });
    ctx.run_one_for(std::chrono::milliseconds{ 10 });
    expect(finished);
  };

  "timer test within awaitable"_test = [] {
    asio::io_context ctx{};
    asio::basic_waitable_timer<tfc::testing::clock, tfc::testing::wait_traits> timer{ ctx };

    bool finished{ false };
    asio::co_spawn(
        ctx,
        [&finished, &timer]() -> asio::awaitable<void> {
          timer.expires_after(std::chrono::days{ 100 });
          timer.async_wait([&finished](std::error_code const& err) {
            if (!err) {
              finished = true;
              return;
            }
            expect(false);
          });
          tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::days{ 100 });
          co_return;
        },
        asio::detached);
    ctx.run_one_for(std::chrono::seconds{ 1 });
    ctx.run_one_for(std::chrono::seconds{ 1 });
    ctx.run_one_for(std::chrono::seconds{ 1 });
    expect(finished);
  };

  return 0;
}
