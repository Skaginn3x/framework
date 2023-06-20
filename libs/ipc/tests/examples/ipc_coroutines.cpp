#include <fmt/core.h>
#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/asio/experimental/use_coro.hpp>

#include <tfc/ipc/details/impl.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;

namespace {
auto timer_coro(asio::steady_timer& timer, std::shared_ptr<tfc::ipc::details::signal<tfc::ipc::details::type_bool>> signal)
    -> asio::awaitable<void> {
  bool send_value{ true };
  while (true) {
    timer.expires_from_now(std::chrono::seconds{ 3 });
    co_await timer.async_wait(asio::use_awaitable);
    co_await signal->coro_send(send_value);
    send_value = !send_value;
  }
}

auto slot_coro(tfc::ipc::details::slot<tfc::ipc::details::type_bool>& slot) -> asio::awaitable<void> {
  while (true) {
    std::expected<bool, std::error_code> msg = co_await slot.coro_receive();
    if (msg) {
      fmt::print("message={}\n", msg.value());
    } else {
      fmt::print("error={}\n", msg.error().message());
    }
  }
}

asio::experimental::coro<int, bool> work(asio::io_context& ctx) {
  asio::steady_timer timer{ ctx };
  timer.expires_from_now(std::chrono::seconds{ 3 });
  co_await timer.async_wait(asio::experimental::use_coro);
  co_yield 42;
  timer.expires_from_now(std::chrono::seconds{ 3 });
  co_await timer.async_wait(asio::experimental::use_coro);
  co_yield 31;
  co_return false;  // done
}

asio::experimental::coro<void, int> testme(asio::io_context& ctx) {
  asio::steady_timer timer{ ctx };
  auto work_awaitable = work(ctx);
  while (true) {
    auto return_val = co_await work_awaitable;
    if (auto* foo = std::get_if<int>(&return_val)) {
      fmt::print("experimental value is: {}\n", *foo);
    } else {  // work done
      fmt::print("Work done\n");
      break;
    }
  }
  co_return 1337;
}

}  // namespace

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::ipc::details::slot<tfc::ipc::details::type_bool> slot{ ctx, "my_name" };

  asio::co_spawn(ctx, slot_coro(slot), asio::detached);

  auto signal{ tfc::ipc::details::signal<tfc::ipc::details::type_bool>::create(ctx, "your_name") };

  slot.connect(signal.value()->name_w_type());

  asio::steady_timer timer{ ctx };

  asio::co_spawn(ctx, timer_coro(timer, signal.value()), asio::detached);

  asio::experimental::co_spawn(testme(ctx), [](std::exception_ptr const&, int return_val) {
    fmt::print("Coroutine is finished with return value: {}\n", return_val);
  });

  ctx.run();

  return EXIT_SUCCESS;
}
