
#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <fmt/core.h>

namespace asio = boost::asio;
using asio::experimental::awaitable_operators::operator||;

inline auto timer(std::chrono::steady_clock::duration dur) -> asio::awaitable<void> {
  asio::steady_timer timer(co_await asio::this_coro::executor);
  timer.expires_after(dur);
  co_await timer.async_wait(asio::use_awaitable);
}

// https://github.com/chriskohlhoff/asio/commit/5e17ca1bbf7878873047287c71e11b8eeebc5ae5
/*
 * template<typename MutableBufferSequence>
struct async_receive_initiation {
  azmq::socket &socket;
  MutableBufferSequence const &buffers;

template<typename CompletionHandler>
void operator()(CompletionHandler &&completion_handler) {
  [[maybe_unused]] auto foo = boost::asio::get_associated_cancellation_slot(completion_handler);

  auto executor = boost::asio::get_associated_executor(
      completion_handler, socket.get_executor());
  socket.async_receive(buffers, boost::asio::bind_executor(executor,
                                                           std::bind(std::forward<CompletionHandler>(completion_handler), std::placeholders::_1, std::placeholders::_2)));
}
};
 */

inline auto tcp_reader(asio::io_context&) -> asio::awaitable<void> {
  auto executor = co_await asio::this_coro::executor;
  asio::ip::udp::socket socket{ executor, { asio::ip::udp::v4(), 1337 } };

  co_await (socket.async_wait(asio::socket_base::wait_type::wait_read, asio::use_awaitable) || timer(std::chrono::seconds{10}));
}

inline auto zmq_reader(asio::io_context& ctx) -> asio::awaitable<void> {
  azmq::sub_socket socket{ ctx };

  std::array<char, 9> buffer{};
  co_await (azmq::async_receive(socket, asio::buffer(buffer), asio::use_awaitable) || timer(std::chrono::seconds{4}) );
}

int main() {
  asio::io_context ctx{};

  asio::co_spawn(ctx, zmq_reader(ctx), [](std::exception_ptr ptr){
    if (ptr) {
      std::rethrow_exception(ptr);
    }
    fmt::print("I am out ! \n ");
    fmt::print("o-\n ");
    fmt::print("|\n ");
    fmt::print("|\n ");
    fmt::print("|\n ");
    fmt::print(".\n ");
    fmt::print(".\n ");
    fmt::print("o-\n ");
    fmt::print("¯\n ");
  });

  ctx.run();

  return 0;
}
