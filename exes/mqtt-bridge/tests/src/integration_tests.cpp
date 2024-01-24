#include <thread>
#include <optional>

// #include <async-mqtt/test/system/broker_running.hpp


auto main() -> int {
    asio::io_context io_ctx{};

    broker_runner broker_i{};

    // io_ctx.run();

    return 0;
}
