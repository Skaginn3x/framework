
#include <boost/asio.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <boost/asio/experimental/co_spawn.hpp>

#include <async_mqtt/all.hpp>

namespace asio = boost::asio;
namespace am = async_mqtt;

static asio::awaitable<void> make_coro(asio::io_context& ctx) {
  asio::ip::tcp::socket resolve_sock{ctx};
  asio::ip::tcp::resolver res{resolve_sock.get_executor()};
  am::endpoint<am::role::client, am::protocol::mqtt> amep {
    am::protocol_version::v3_1_1,
    ctx.get_executor()
  };
  asio::ip::tcp::resolver::results_type resolved_ip = co_await res.async_resolve("localhost", "1883", asio::use_awaitable);

  [[maybe_unused]] asio::ip::tcp::endpoint endpoint = co_await asio::async_connect(amep.next_layer(), resolved_ip, asio::use_awaitable);

  co_await amep.send(am::v3_1_1::connect_packet{
      true,   // clean_session
      0x1234, // keep_alive
      am::allocate_buffer("cid1"),
      am::nullopt, // will
      am::nullopt, // username set like am::allocate_buffer("user1"),
      am::nullopt  // password set like am::allocate_buffer("pass1")
  }, asio::use_awaitable);

  [[maybe_unused]] am::packet_variant packet_variant = co_await amep.recv(asio::use_awaitable);
  // todo do something with the above packet

  co_await amep.send(am::v3_1_1::publish_packet{ *amep.acquire_unique_packet_id(), am::allocate_buffer("signal_topic"),
                                        am::allocate_buffer("signal_payload"), am::qos::at_least_once }, asio::use_awaitable);

}


int main (int argc, char* argv[]) {

  asio::io_context ctx{};

  asio::co_spawn(ctx, make_coro(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}

