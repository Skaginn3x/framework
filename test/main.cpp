#include <connection.hpp>
#include <boost/asio.hpp>

auto main(int, char**) -> int {
  boost::asio::io_context ctx{};

  [[maybe_unused]] connection_test testme{ctx};

  ctx.run();
  return EXIT_SUCCESS;
}
