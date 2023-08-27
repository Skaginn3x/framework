#pragma once

#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>

using org::eclipse::tahu::protobuf::Payload;
using org::eclipse::tahu::protobuf::Payload_Metric;

namespace asio = boost::asio;

// This class mocks the response to send for the MQTT client
class result {
public:
  explicit result(std::string& message) : message_(message) {}

  explicit operator bool() const { return message_ != "Success"; }

  [[nodiscard]] auto message() -> std::string { return message_; }

private:
  std::string message_;
};

class next_layer_mock {
public:
  auto native_handle() -> int { return 10; }
};

// This class mocks the behaviour that the MQTT client would have
template <class role, class protocol>
class mock_mqtt_client {
public:
  mock_mqtt_client(async_mqtt::protocol_version,
                   const asio::io_context::executor_type& executor,
                   const async_mqtt::tls::context&)
      : ssl_ctx_(asio::ssl::context::sslv23), strand_(executor), ctx_(executor) {}

  auto strand() -> asio::strand<asio::io_context::executor_type>& { return strand_; }

  // These functions don't matter, they just need to return something
  auto lowest_layer() -> int { return 10; }

  auto next_layer() -> next_layer_mock { return next_layer_; }

  // This function sends a publishing packet (normal packet) to the mock client
  auto send(async_mqtt::v5::publish_packet packet, const asio::use_awaitable_t<>&) -> asio::awaitable<result> {
    messages_.push_back(packet);
    std::string message = "Success";
    co_return result{ message };
  }

  // This function sends a connect packet to the mock client
  auto send(async_mqtt::v5::connect_packet packet, const asio::use_awaitable_t<>&) -> asio::awaitable<result> {
    messages_.push_back(packet);
    std::string message = "Success";
    co_return result{ message };
  }

  // This function sends a subscribe packet to the mock client
  auto send(async_mqtt::v5::subscribe_packet packet, const asio::use_awaitable_t<>&) -> asio::awaitable<result> {
    messages_.push_back(packet);
    std::string message = "Success";
    co_return result{ message };
  }

  // This function returns the last packet that was sent to the mock client
  auto get_last_message()
      -> std::variant<async_mqtt::v5::publish_packet, async_mqtt::v5::connect_packet, async_mqtt::v5::subscribe_packet> {
    if (messages_.empty()) {
      throw std::exception();
    }
    return messages_.back();
  }

  // This function returns a unique packet ID that is consecutively incremented
  auto acquire_unique_packet_id() -> std::optional<uint16_t> { return packet_id_++; }

  // This function should be called to close the socket of the MQTT client but for mocking purposes it is not strictly needed
  static auto close(asio::use_awaitable_t<>) -> asio::awaitable<void> { co_return; }

  // Caller wants to wait for a specific packet to arrive
  auto recv(async_mqtt::filter, std::set<async_mqtt::control_packet_type> packet_type, asio::use_awaitable_t<>)
      -> asio::awaitable<async_mqtt::packet_variant> {
    if (!packet_type.empty()) {
      auto firstElement = *packet_type.begin();
      if (firstElement == async_mqtt::control_packet_type::publish) {
        Payload payload;

        auto* metric = payload.add_metrics();
        metric->set_name("Node Control/Rebirth");

        payload.set_timestamp(123);

        std::string payload_string;
        payload.SerializeToString(&payload_string);

        co_return async_mqtt::v5::publish_packet{ 0, async_mqtt::allocate_buffer("NCMD"),
                                                  async_mqtt::allocate_buffer(payload_string),
                                                  async_mqtt::qos::at_most_once | async_mqtt::pub::retain::no };
      } else if (firstElement == async_mqtt::control_packet_type::suback) {
        co_return async_mqtt::v5::suback_packet{ 0,
                                                 { async_mqtt::suback_reason_code::granted_qos_0 },
                                                 async_mqtt::properties{} };
      } else if (firstElement == async_mqtt::control_packet_type::connack) {
        co_return async_mqtt::v5::connack_packet{ true, async_mqtt::connect_reason_code::success };
      }
    }
    co_return async_mqtt::v5::connack_packet{ false, async_mqtt::connect_reason_code::server_unavailable };
  }

private:
  asio::ssl::context ssl_ctx_;
  asio::strand<asio::io_context::executor_type> strand_;
  asio::io_context::executor_type ctx_;

  std::vector<
      std::variant<async_mqtt::v5::publish_packet, async_mqtt::v5::connect_packet, async_mqtt::v5::subscribe_packet> >
      messages_;

  uint16_t packet_id_ = 0;

  next_layer_mock next_layer_ = next_layer();
};

struct SparkplugBConfig {
  std::string address;
  uint16_t port;
  std::string username;
  std::string password;
  std::string node_id;
  std::string group_id;
  std::string client_id;
};

class mock_config {
  SparkplugBConfig _sparkplug_b;

public:
  mock_config(asio::io_context&, std::string) {
    _sparkplug_b.address = "localhost";
    _sparkplug_b.port = 1883;
    _sparkplug_b.username = "username";
    _sparkplug_b.password = "password";
    _sparkplug_b.node_id = "edge_node_id";
    _sparkplug_b.group_id = "group_id";
    _sparkplug_b.client_id = "client_id";
  }

  SparkplugBConfig value() { return _sparkplug_b; }
};

class network_manager_mock {
public:
  auto connect_socket(auto&&, auto&&) -> asio::awaitable<void> { co_return; }

  auto set_sni_hostname(auto&&, std::string) -> void { return; }

  auto async_handshake(auto&&) -> asio::awaitable<void> { co_return; }
};
