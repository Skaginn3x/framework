#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <typeindex>
#include <utility>

#include <async_mqtt/all.hpp>
#include <async_mqtt/predefined_layer/mqtts.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>

#include <structs.hpp>

template <typename... types>
class package_v {
  std::type_index type_index_t = typeid(void);
  std::tuple<std::unique_ptr<types>...> data;

public:
  package_v() = default;

  template <typename T>
  void set(const T& value) {
    type_index_t = std::type_index(typeid(T));
    std::get<std::unique_ptr<T>>(data) = std::make_unique<T>(value);
  }

  template <typename T>
  T& get() {
    if (type_index_t != std::type_index(typeid(T))) {
      throw std::runtime_error("Bad MyVariant access");
    }
    return *(std::get<std::unique_ptr<T>>(data));
  }

  template <typename T>
  const T& get() const {
    if (type_index_t != std::type_index(typeid(T))) {
      throw std::runtime_error("Bad MyVariant access");
    }
    return *(std::get<std::unique_ptr<T>>(data));
  }

  template <typename T>
  T* get_if() {
    if (type_index_t == std::type_index(typeid(T))) {
      return std::get<std::unique_ptr<T>>(data).get();
    }
    return nullptr;
  }
};

namespace tfc::mqtt {

namespace asio = boost::asio;

using boost::asio::experimental::awaitable_operators::operator||;

class endpoint_client_mock {
public:
  explicit endpoint_client_mock(asio::io_context& ctx, structs::ssl_active_e) : ctx_(ctx) {}

  auto get_executor() const -> asio::any_io_executor { return ctx_.get_executor(); }

  auto recv(async_mqtt::control_packet_type packet_t) -> asio::awaitable<
      package_v<async_mqtt::v5::suback_packet, async_mqtt::v5::publish_packet, async_mqtt::v5::connack_packet>> {
    if (packet_t == async_mqtt::control_packet_type::suback) {
      package_v<async_mqtt::v5::suback_packet, async_mqtt::v5::publish_packet, async_mqtt::v5::connack_packet> my_variant;
      my_variant.set<async_mqtt::v5::suback_packet>(
          async_mqtt::v5::suback_packet{ 0, { async_mqtt::suback_reason_code::granted_qos_0 } });
      co_return my_variant;
    } else if (packet_t == async_mqtt::control_packet_type::publish) {
      package_v<async_mqtt::v5::suback_packet, async_mqtt::v5::publish_packet, async_mqtt::v5::connack_packet> my_variant;
      my_variant.set<async_mqtt::v5::publish_packet>(async_mqtt::v5::publish_packet{
          0, "topic", "payload", { async_mqtt::pub::retain::no }, async_mqtt::properties{} });
      co_return my_variant;
    }
    package_v<async_mqtt::v5::suback_packet, async_mqtt::v5::publish_packet, async_mqtt::v5::connack_packet> my_variant;
    my_variant.set<async_mqtt::v5::connack_packet>(
        async_mqtt::v5::connack_packet{ true, async_mqtt::connect_reason_code::success });
    co_return my_variant;
  }

  template <typename... args_t>
  auto send(args_t&&...) -> asio::awaitable<std::tuple<std::error_code>> {
    co_return std::make_tuple(std::error_code{});
  }

  template <typename... args_t>
  auto close(args_t&&...) {
    // mocking closing the socket in the real client
  }

  auto acquire_unique_packet_id() -> std::optional<uint16_t> { return 0; }

  auto async_connect(asio::ip::tcp::resolver::results_type) -> asio::awaitable<void> { co_return; }

  template <typename client_t>
  auto async_connect_loop(client_t&, asio::ip::tcp::resolver::results_type) -> asio::awaitable<void> {
    co_return;
  }

  auto set_sni_hostname(std::string_view) -> void {
    // mocking setting the sni hostname of the ssl broker
  }

  auto async_handshake() -> asio::awaitable<void> { co_return; }

private:
  asio::io_context& ctx_;
  async_mqtt::tls::context tls_ctx_{ async_mqtt::tls::context::tlsv12 };
  std::optional<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>> mqtt_client_;
  std::optional<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>> mqtts_client_;
};

}  // namespace tfc::mqtt
