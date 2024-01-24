#include <thread>
#include <optional>

#include <boost/asio.hpp>

#include <broker/test/system/broker_runner.hpp>

namespace asio = boost::asio;

auto main() -> int {
    asio::io_context io_ctx{};

    broker_runner broker_i{};

    io_ctx.run();

    return 0;
}
