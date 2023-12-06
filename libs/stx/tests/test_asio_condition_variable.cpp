#include <chrono>
#include <utility>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/ut.hpp>

#include <tfc/utils/asio_condition_variable.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::fatal;
using ut::operator>>;

struct test_instance {
  asio::io_context ctx;
  tfc::asio::condition_variable<asio::any_io_executor> cv{ ctx.get_executor() };
  bool called{};
  bool called2{};

  void async_wait(bool& was_called) {
    cv.async_wait([&was_called](std::error_code const& err) {
      ut::expect(!err) << err.message();
      was_called = true;
    });
  }
};

auto main() -> int {
  using std::chrono_literals::operator""ms;

  "one handler"_test = [] {
    test_instance test{};
    test.async_wait(test.called);
    test.ctx.run_for(1ms);
    test.cv.notify_one();
    test.ctx.run_for(1ms);
    ut::expect(test.called);
  };

  "one handler moved"_test = [] {
    test_instance test{};
    test.async_wait(test.called);
    auto cv2{ std::move(test.cv) };
    test.ctx.run_for(1ms);
    cv2.notify_one();
    auto cv3{ std::move(cv2) };
    test.ctx.run_for(1ms);
    ut::expect(test.called);
  };

  "two handlers"_test = [] {
    test_instance test{};
    test.async_wait(test.called);
    test.async_wait(test.called2);
    test.ctx.run_for(1ms);
    test.cv.notify_one();
    test.ctx.run_for(1ms);
    ut::expect(test.called);
    ut::expect(!test.called2);
    test.cv.notify_one();
    test.ctx.run_for(1ms);
    ut::expect(test.called2);
  };

  "two handlers same event cycle"_test = [] {
    test_instance test{};
    test.async_wait(test.called);
    test.async_wait(test.called2);
    test.ctx.run_for(1ms);
    test.cv.notify_one();
    test.cv.notify_one();
    test.ctx.run_for(1ms);
    ut::expect(test.called);
    ut::expect(test.called2);
  };

  "two handlers notify all"_test = [] {
    test_instance test{};
    test.async_wait(test.called);
    test.async_wait(test.called2);
    test.ctx.run_for(1ms);
    test.cv.notify_all();
    test.ctx.run_for(1ms);
    ut::expect(test.called);
    ut::expect(test.called2);
  };

  "two CVs but only one activated"_test = [] {
    asio::io_context ctx;
    tfc::asio::condition_variable cv1{ ctx.get_executor() };
    tfc::asio::condition_variable cv2{ ctx.get_executor() };
    bool called{};
    asio::co_spawn(
        ctx,
        [&cv1, &cv2, &called]() -> asio::awaitable<void> {
          using asio::experimental::awaitable_operators::operator||;
          auto err_variant{ co_await (cv1.async_wait(asio::use_awaitable) || cv2.async_wait(asio::use_awaitable)) };
          auto err{ std::get<1>(err_variant) };  // cv2 is notified
          ut::expect(!err) << err.message();
          called = true;
          co_return;
        },
        asio::detached);
    ctx.run_for(1ms);
    cv2.notify_one();
    ctx.run_for(1ms);
    ut::expect(called);
  };

  return EXIT_SUCCESS;
}
