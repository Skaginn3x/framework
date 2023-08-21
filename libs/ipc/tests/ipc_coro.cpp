
#include <boost/asio.hpp>
#include <fmt/core.h>

namespace asio = boost::asio;

auto reader() -> asio::awaitable<void> {
  auto executor = co_await asio::this_coro::executor;
  asio::ip::tcp::socket socket{ executor };


}

int main() {
  asio::io_context ctx{};

  asio::co_spawn(ctx, reader(), [](std::exception_ptr ptr){
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
    fmt::print("_\n ");
  });

  ctx.run();

  return 0;
}
