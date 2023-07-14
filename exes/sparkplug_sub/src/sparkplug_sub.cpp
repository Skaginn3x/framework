/*
 * Connect to a broker and print sparkplug-b messages
 * */
#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;

template <typename endpoint_t>
struct mqtt_connection {
  mqtt_connection(asio::io_context& ctx,
                  const std::string_view username,
                  const std::string_view password,
                  const std::string_view hostname,
                  const std::string_view port_number,
                  endpoint_t& ep)
      : sock_{ ctx }, resolver_{ ctx }, ep_{ ep }, username_{ username }, password_{ password }, hostname_{ hostname },
        port_number_{ port_number } {}
  void start() {
    resolver_.async_resolve(hostname_, port_number_, std::bind_front(&mqtt_connection::resolve_callback, this));
  }
  asio::ip::tcp::socket sock_;
  void resolve_callback(const std::error_code& err, const asio::ip::tcp::resolver::results_type results) {
    if (err) {
      logger_.error("resolve_callback: {}", err.message());
      return;
    }
    logger_.trace("Resolve endpoint {}:{}", results->endpoint().address().to_string(), results->endpoint().port());
    asio::async_connect(ep_.next_layer().next_layer(), results, std::bind_front(&mqtt_connection::connect_callback, this));
  }
  void connect_callback(const std::error_code& err, const asio::ip::tcp::endpoint& endpoint) {
    if (err) {
      logger_.error("connect_callback: {}", err.message());
      return;
    }
    logger_.trace("Connected to {}:{}", endpoint.address().to_string(), endpoint.port());
    ep_.next_layer().async_handshake(async_mqtt::tls::stream_base::client,
                                     std::bind_front(&mqtt_connection::handshake_callback, this));
  }
  void handshake_callback(const std::error_code& err) {
    if (err) {
      logger_.error("handshake_callback: {}", err.message());
      return;
    }
    char hostname[256];
    gethostname(hostname, 256);
    ep_.send(async_mqtt::v3_1_1::connect_packet{ true, 50, async_mqtt::allocate_buffer(fmt::format("client-{}", hostname)),
                                                 async_mqtt::nullopt, async_mqtt::buffer(username_), async_mqtt::buffer(password_) },
             std::bind_front(&mqtt_connection::send_callback, this));
  }
  void send_callback(const async_mqtt::system_error& err) {
    if (err) {
      logger_.error("send_callback: {}", err.message());
      return;
    }
    logger_.trace("Sent connect packet");
    ep_.recv(std::bind_front(&mqtt_connection::recv_callback, this));
  }
  void recv_callback(const async_mqtt::packet_variant& pv) {
    logger_.trace("Received packet");
    pv.visit(async_mqtt::overload{ [&](async_mqtt::v3_1_1::connack_packet const& connack_packet) {
                                    logger_.trace("Connack Packet recieved\nSession present {} Code {}",
                                                  connack_packet.session_present(),
                                                  static_cast<uint8_t>(connack_packet.code())
                                                  );
                                  },
                                   [](auto const&) {} });
  }
  asio::ip::tcp::resolver resolver_;
  endpoint_t& ep_;
  tfc::logger::logger logger_{ "mqtt_connection" };
  const std::string_view username_;
  const std::string_view password_;
  const std::string_view hostname_;
  const std::string_view port_number_;
};

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };

  std::string username;
  std::string password;
  std::string hostname;
  std::string port_number;

  std::vector<std::string> topics{ "spBv1.0/+/DDATA/+/+" };

  description.add_options()("username,u", po::value<std::string>(&username), "Username")(
      "port-number,p", po::value<std::string>(&port_number), "Port number")("password,P", po::value<std::string>(&password),
                                                                            "Password")(
      "topic,t", po::value<std::vector<std::string>>(&topics)->multitoken(), "Message topic")(
      "hostname", po::value<std::string>(&hostname), "Hostname");
  tfc::base::init(argc, argv, description);

  tfc::logger::logger logger{ "main" };

  logger.info("username: {}", username);
  logger.info("password: {}", password);
  logger.info("hostname: {}", hostname);
  logger.info("port_number: {}", port_number);
  for (auto& topic : topics) {
    logger.info("topic: {}", topic);
  }

  asio::io_context ctx;

  async_mqtt::tls::context tls_ctx{ async_mqtt::tls::context::tls };

  // verify - dont verify
  tls_ctx.set_verify_mode(asio::ssl::verify_none);

  async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts> ep{ async_mqtt::protocol_version::v3_1_1, ctx,
                                                                                  tls_ctx };

  mqtt_connection mqc(ctx, username, password, hostname, port_number, ep);
  mqc.start();
  ctx.run();
  return 0;
}
