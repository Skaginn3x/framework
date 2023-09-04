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
    asio::basic_waitable_timer<tfc::testing::clock> timer{ ctx };
    timer.expires_after(std::chrono::milliseconds{ 1 });
    bool finished{ false };
    timer.async_wait([&finished](std::error_code const& err) {
      if (!err) {
        finished = true;
        return;
      }
      expect(false);
    });
    tfc::testing::clock::set_ticks(tfc::testing::clock::now() + std::chrono::milliseconds{ 1 });
    ctx.run_one_for(std::chrono::milliseconds{10});
    expect(finished);
  };
  return 0;
}
