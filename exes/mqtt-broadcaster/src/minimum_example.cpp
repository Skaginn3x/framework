#include <async_mqtt/all.hpp>
#include <tfc/ipc.hpp>
#include "mqtt_broadcaster.hpp"

#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iterator>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/unpack_properties.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/to_tuple.hpp>

// using namespace std;
namespace asio = boost::asio;

class test_class {
private:
  asio::io_context& ctx_;
  std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>> mqtt_client;

public:
  test_class(asio::io_context& ctx)
      : ctx_(ctx), mqtt_client(std::make_shared<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>(
                       async_mqtt::protocol_version::v3_1_1,
                       ctx_.get_executor())) {
    asio::co_spawn(mqtt_client->strand(), test_func(), asio::detached);
  }

  auto test_func() -> asio::awaitable<void> {
    asio::ip::tcp::socket resolve_sock{ ctx_ };
    asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
    asio::ip::tcp::resolver::results_type resolved_ip = co_await res.async_resolve("localhost", "1883", asio::use_awaitable);

    try {
      asio::ip::tcp::endpoint endpoint =
          co_await asio::async_connect(mqtt_client->next_layer(), resolved_ip, asio::use_awaitable);

      std::cout << endpoint.address().to_string() << '\n';
    } catch (const std::exception& e) {
      std::cout << "Exception during connect: " << e.what() << '\n';
    } catch (...) {
      std::cout << "Unknown exception during connect\n";
    }

    co_await mqtt_client->send(
        async_mqtt::v3_1_1::connect_packet{ false, 0x1234, async_mqtt::allocate_buffer("cid1"), async_mqtt::nullopt,
                                            async_mqtt::nullopt, async_mqtt::nullopt },
        asio::use_awaitable);

    async_mqtt::packet_variant packet_variant = co_await mqtt_client->recv(asio::use_awaitable);

    std::cout << "im here\n";
    std::cout << "im here\n";
    // std::cout << "result: " << packet_variant.get<packet_variant.type()>().packet_id() << "\n";
    std::cout << "im here\n";

    co_await mqtt_client->send(
        async_mqtt::v3_1_1::publish_packet{ mqtt_client->acquire_unique_packet_id().value(),
                                            async_mqtt::allocate_buffer("test_topic"),
                                            async_mqtt::allocate_buffer("test_payload"), async_mqtt::qos::at_least_once },
        asio::use_awaitable);

    std::string payload = "n";
    while (true) {
      std::cout << "Sending payload: " << payload << "\n";

      auto packet_id = mqtt_client->acquire_unique_packet_id().value();

      auto result = co_await mqtt_client->send(
          async_mqtt::v3_1_1::publish_packet{ packet_id, async_mqtt::allocate_buffer("test_topic"),
                                              async_mqtt::allocate_buffer(payload), async_mqtt::qos::at_least_once },
          asio::use_awaitable);

      if (!result) {
        std::cout << "Successfully sent payload\n";
      } else {
        std::cout << "Failed to send payload\n";
      }

      asio::steady_timer timer{ ctx_, std::chrono::seconds{ 3 } };
      co_await timer.async_wait(asio::use_awaitable);
      payload += "n";
    }
    co_return;
  }
};

auto main() -> int {
  asio::io_context ctx{};
  test_class tester(ctx);
  ctx.run();
  return 0;
}
