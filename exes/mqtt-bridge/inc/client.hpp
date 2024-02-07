#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include <boost/asio.hpp>

#include <config/bridge.hpp>
#include <config/bridge_mock.hpp>
#include <tfc/confman.hpp>
#include <tfc/logger.hpp>
#include <tfc/utils/asio_fwd.hpp>

namespace async_mqtt {
enum class qos : std::uint8_t;

class buffer;
namespace v5 {

class connect_packet;

template <std::size_t PacketIdBytes>
class basic_publish_packet;

using publish_packet = basic_publish_packet<2>;
}  // namespace v5
}  // namespace async_mqtt

namespace tfc::mqtt {

class endpoint_client;
class endpoint_client_mock;

namespace asio = boost::asio;

template <class client_t, class config_t>
class client {
public:
  explicit client(asio::io_context& io_ctx,
                  std::string_view mqtt_will_topic,
                  std::string_view mqtt_will_payload,
                  config_t& config);

  auto connect() -> asio::awaitable<bool>;

  auto connect_and_handshake(asio::ip::tcp::resolver::results_type resolved_ip) -> asio::awaitable<bool>;

  auto receive_connack() -> asio::awaitable<bool>;

  auto send_message(std::string topic, std::string payload, async_mqtt::qos qos) -> asio::awaitable<bool>;

  auto subscribe_to_topic(std::string topic) -> asio::awaitable<bool>;

  auto wait_for_payloads(
      std::function<void(async_mqtt::buffer const& data, async_mqtt::v5::publish_packet publish_packet)> process_payload,
      bool& restart_needed) -> asio::awaitable<void>;

  auto strand() -> asio::strand<asio::any_io_executor>;

  auto set_initial_message(std::string const& topic, std::string const& payload, async_mqtt::qos const& qos) -> void;

  auto send_initial() -> asio::awaitable<bool>;

  auto connect_packet() -> async_mqtt::v5::connect_packet;

private:
  asio::io_context& io_ctx_;
  std::string mqtt_will_topic_;
  std::string mqtt_will_payload_;
  std::unique_ptr<client_t, std::function<void(client_t*)>> endpoint_client_;
  config_t& config_;
  logger::logger logger_{ "client" };
  std::tuple<std::string, std::string, async_mqtt::qos> initial_message_;
};

using client_n = client<endpoint_client, confman::config<config::bridge>>;
using client_semi_normal = client<endpoint_client, config::bridge_mock>;
using client_mock = client<endpoint_client_mock, config::bridge_mock>;

extern template class client<endpoint_client, confman::config<config::bridge>>;
extern template class client<endpoint_client, config::bridge_mock>;
extern template class client<endpoint_client_mock, config::bridge_mock>;

}  // namespace tfc::mqtt
