#include <async_mqtt/all.hpp>
#include <tfc/ipc.hpp>
#include "mqtt_broadcaster.hpp"

auto try_connect(auto mqtt_client, auto completion_token) {
  asio::ip::tcp::socket resolve_sock{ ctx };
  asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
  res.async_resolve("localhost", "1883", [mqtt_client](std::error_code, asio::ip::tcp::resolver::results_type const& resolved_ip){
    asio::async_connect(mqtt_client->next_layer(), resolved_ip, [mqtt_client](std::error_code err0, asio::ip::tcp::endpoint const& ){
      fmt::print("connect error: {}\n", err0.message());
      mqtt_client->send(
          async_mqtt::v3_1_1::connect_packet{ false, 0x1234, async_mqtt::allocate_buffer("cid1"), std::nullopt,
                                              std::nullopt, std::nullopt },
          [mqtt_client](async_mqtt::system_error const& err1){
            fmt::print("Sending error: {}\n", err1.message());
            mqtt_client->recv([mqtt_client]([[maybe_unused]] async_mqtt::packet_variant const& var){
              fmt::print("Receive packet\n");
            });
          });
    });
    mqtt_client->next_layer().set_option(asio::socket_base::keep_alive(true));
  });

}

// TODO: if nothing is printed when the program is started, ipc-ruler is not running
auto main(int argc, char* argv[]) -> int {
  // try {
  auto program_description{ tfc::base::default_description() };

//  std::string mqtt_host;
//  std::string mqtt_port;
//  std::string mqtt_username;
//  std::string mqtt_password;

//  program_description.add_options()("mqtt_host", boost::program_options::value<std::string>(&mqtt_host)->required(),
//                                    "ip address of mqtt broker")(
//      "mqtt_port", boost::program_options::value<std::string>(&mqtt_port)->required(), "port of mqtt broker")(
//      "mqtt_username", boost::program_options::value<std::string>(&mqtt_username), "username of mqtt broker")(
//      "mqtt_password", boost::program_options::value<std::string>(&mqtt_password), "password of mqtt broker");

  tfc::base::init(argc, argv, program_description);

  asio::io_context ctx{};

//  tfc::ipc_ruler::ipc_manager_client ipc_client{ ctx };

  const std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>> mqtt_client =
      std::make_shared<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>(
          async_mqtt::protocol_version::v3_1_1, ctx.get_executor());

  asio::ip::tcp::socket resolve_sock{ ctx };
  asio::ip::tcp::resolver res{ resolve_sock.get_executor() };
  res.async_resolve("localhost", "1883", [mqtt_client](std::error_code, asio::ip::tcp::resolver::results_type const& resolved_ip){
    asio::async_connect(mqtt_client->next_layer(), resolved_ip, [mqtt_client](std::error_code err0, asio::ip::tcp::endpoint const& ){
      fmt::print("connect error: {}\n", err0.message());
      mqtt_client->send(
        async_mqtt::v3_1_1::connect_packet{ false, 0x1234, async_mqtt::allocate_buffer("cid1"), std::nullopt,
                                            std::nullopt, std::nullopt },
      [mqtt_client](async_mqtt::system_error const& err1){
            fmt::print("Sending error: {}\n", err1.message());
            mqtt_client->recv([mqtt_client]([[maybe_unused]] async_mqtt::packet_variant const& var){
              fmt::print("Receive packet\n");
            });
          });
    });
    mqtt_client->next_layer().set_option(asio::socket_base::keep_alive(true));
  });

//  [[maybe_unused]] asio::ip::tcp::endpoint endpoint =
//      co_await asio::async_connect(mqtt_client_->next_layer(), resolved_ip, asio::use_awaitable);
//
//  co_await mqtt_client_->send(
//      async_mqtt::v3_1_1::connect_packet{ false, 0x1234, async_mqtt::allocate_buffer("cid1"), async_mqtt::nullopt,
//                                          async_mqtt::allocate_buffer(mqtt_username_), async_mqtt::allocate_buffer(mqtt_password_) },
//      asio::use_awaitable);
//
//  async_mqtt::packet_variant packet_variant = co_await mqtt_client_->recv(asio::use_awaitable);
//
//  co_await mqtt_client_->send(async_mqtt::v3_1_1::publish_packet{ mqtt_client_->acquire_unique_packet_id().value(),
//                                                                  async_mqtt::allocate_buffer("test_topic"),
//                                                                  async_mqtt::allocate_buffer("test_payload"),
//                                                                  async_mqtt::qos::at_least_once },
//                              asio::use_awaitable);

//  mqtt_broadcaster<tfc::ipc_ruler::ipc_manager_client&, sdbusplus::bus::match::match,
//                   async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>>
//      application(ctx, mqtt_host, mqtt_port, mqtt_username, mqtt_password, ipc_client, mqtt_client);

  ctx.run();

  return 0;
  //} catch (const std::exception& e) {
  //    std::cerr << "Exception caught: " << e.what() << '\n';
  //    return 1;  // or any other non-zero exit code
  //  } catch (...) {
  //    std::cerr << "Unknown exception caught\n";
  //    return 2;  // or any other non-zero exit code
  //  }
}
