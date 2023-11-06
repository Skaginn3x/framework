#pragma once

#include <config/broker.hpp>
#include <config/broker_mock.hpp>
#include <tfc/confman.hpp>
#include <tfc/logger.hpp>

import std;
import asio;

namespace async_mqtt {
enum class qos : std::uint8_t;

class buffer;
namespace v5 {
template <std::size_t PacketIdBytes>
class basic_publish_packet;

using publish_packet = basic_publish_packet<2>;
}  // namespace v5
}  // namespace async_mqtt

namespace tfc::mqtt {

class endpoint_client;
class endpoint_client_mock;

template <class client_t, class config_t>
class client {
public:
  explicit client(asio::io_context& io_ctx, std::string_view mqtt_will_topic, std::string_view mqtt_will_payload);

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

private:
  asio::io_context& io_ctx_;
  std::string mqtt_will_topic_;
  std::string mqtt_will_payload_;
  std::unique_ptr<client_t, std::function<void(client_t*)>> endpoint_client_;
  config_t config_{ io_ctx_, "client" };
  tfc::logger::logger logger_{ "client" };
  std::tuple<std::string, std::string, async_mqtt::qos> initial_message_;
};

using client_n = client<endpoint_client, tfc::confman::config<config::broker>>;
using client_mock = client<endpoint_client_mock, config::broker_mock>;

extern template class tfc::mqtt::client<tfc::mqtt::endpoint_client, tfc::confman::config<tfc::mqtt::config::broker>>;

extern template class tfc::mqtt::client<tfc::mqtt::endpoint_client_mock, tfc::mqtt::config::broker_mock>;

}  // namespace tfc::mqtt
