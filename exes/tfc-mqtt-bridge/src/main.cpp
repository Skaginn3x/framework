#include <fmt/format.h>
#include <boost/asio.hpp>
#include <async_mqtt/all.hpp>

namespace asio = boost::asio;

struct mqtt_connection {
  mqtt_connection(asio::io_context& ctx): ctx_(ctx) {
    do_reconnect();
  }
  void do_reconnect() {
    // todo check if waiting
    reconnect_timer.expires_from_now(std::chrono::seconds(3));
    reconnect_timer.async_wait(std::bind_front(&mqtt_connection::reconnect_cb, this));
  }
  void mqtt_receive_cb([[maybe_unused]] async_mqtt::packet_variant const& var) {
    fmt::print("Receive packet\n");
    if (auto* err { var.get_if<async_mqtt::system_error>() }) {
      fmt::print("got receive error: {}\n", err->message());
      asio::post(mqtt_client_->strand(), [this](){
        mqtt_client_->send(
            async_mqtt::v5::publish_packet{ mqtt_client_->acquire_unique_packet_id().value(),
                                                async_mqtt::allocate_buffer("test_topic"),
                                                async_mqtt::allocate_buffer("test_payload_disconnect"), async_mqtt::qos::at_least_once },
            [](async_mqtt::system_error const& errfo){
              fmt::print("sent payload, err: {}\n", errfo.message());
            });

        asio::post(mqtt_client_->strand(), [this](){
          mqtt_client_->send(
              async_mqtt::v5::publish_packet{ mqtt_client_->acquire_unique_packet_id().value(),
                                                  async_mqtt::allocate_buffer("test_topic"),
                                                  async_mqtt::allocate_buffer("test_payload_disconnect"), async_mqtt::qos::at_least_once },
              [](async_mqtt::system_error const& errfo){
                fmt::print("sent payload, err: {}\n", errfo.message());
              });
        });
      });
      return do_reconnect();
    }
    else if (auto* puback{ var.get_if<async_mqtt::v5::puback_packet>() }) {

    }
    else {
      mqtt_client_->send(
          async_mqtt::v5::publish_packet{ mqtt_client_->acquire_unique_packet_id().value(),
                                              async_mqtt::allocate_buffer("test_topic"),
                                              async_mqtt::allocate_buffer("test_payload"), async_mqtt::qos::at_least_once },
          [](async_mqtt::system_error const& errfo){
            fmt::print("sent payload, err: {}\n", errfo.message());
          });
    }
    mqtt_client_->recv(std::bind_front(&mqtt_connection::mqtt_receive_cb, this));
  }
  void mqtt_connect_cb(async_mqtt::system_error const& err) {
    fmt::print("Sending error: {}\n", err.message());
    if (err) {
      return do_reconnect();
    }
    mqtt_client_->recv(std::bind_front(&mqtt_connection::mqtt_receive_cb, this));
  }
  void connect_cb(std::error_code err, asio::ip::tcp::endpoint const& ) {
    fmt::print("connect error: {}\n", err.message());
    if (err) {
      return do_reconnect();
    }
    mqtt_client_->next_layer().set_option(asio::socket_base::keep_alive(true));
    mqtt_client_->send(
        async_mqtt::v5::connect_packet{ false, 10, async_mqtt::allocate_buffer("cid1"), std::nullopt,
                                            std::nullopt, std::nullopt, {async_mqtt::property::session_expiry_interval{10000}} }, std::bind_front(&mqtt_connection::mqtt_connect_cb, this)
    );
  }
  void resolve_cb(std::error_code err, asio::ip::tcp::resolver::results_type const& resolved_ip) {
    fmt::print("resolve error: {}\n", err.message());
    if (err) {
      return do_reconnect();
    }
    asio::async_connect(mqtt_client_->next_layer(), resolved_ip, std::bind_front(&mqtt_connection::connect_cb, this));
    mqtt_client_->next_layer().set_option(asio::socket_base::keep_alive(true));
  }
  void reconnect_cb(std::error_code err) {
    if (err) {
      fmt::print("reconnect error: {}\n", err.message());
    }
    else {
      res_.async_resolve("localhost", "1234", std::bind_front(&mqtt_connection::resolve_cb, this));
    }
  }

  asio::io_context& ctx_;
  asio::ip::tcp::socket resolve_sock_{ ctx_ };
  asio::ip::tcp::resolver res_{ resolve_sock_.get_executor() };
  using mqtt_endpoint = async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>;
  std::shared_ptr<mqtt_endpoint> mqtt_client_{ std::make_shared<mqtt_endpoint>(async_mqtt::protocol_version::v5, ctx_.get_executor()) };
  asio::steady_timer reconnect_timer{ ctx_ };

};

auto main() -> int {
  asio::io_context ctx{};

  auto const unused{ mqtt_connection(ctx) };

  ctx.run();

  return 0;
}
