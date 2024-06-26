#pragma once

#include <chrono>
#include <optional>
#include <string_view>
#include <utility>

#include <openssl/ssl.h>
#include <async_mqtt/all.hpp>
#include <async_mqtt/endpoint.hpp>
#include <async_mqtt/packet/control_packet_type.hpp>
#include <async_mqtt/predefined_layer/mqtt.hpp>
#include <async_mqtt/predefined_layer/mqtts.hpp>
#include <async_mqtt/protocol_version.hpp>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/system.hpp>

#include <structs.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

using boost::asio::experimental::awaitable_operators::operator||;

class endpoint_client {
public:
  explicit endpoint_client(asio::io_context& ctx, structs::ssl_active_e input_ssl) : io_ctx_(ctx) {
    if (input_ssl == structs::ssl_active_e::yes) {
      mqtts_client_ = async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>::create(
          async_mqtt::protocol_version::v5, ctx.get_executor(), tls_ctx_);
    } else {
      mqtt_client_ = async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>::create(
          async_mqtt::protocol_version::v5, ctx.get_executor());
    }
  }

  auto get_executor() const -> asio::any_io_executor {
    if (mqtts_client_) {
      return mqtts_client_->get_executor();
    }
    return mqtt_client_->get_executor();
  }

  auto recv(async_mqtt::control_packet_type packet_t) {
    if (mqtts_client_) {
      return mqtts_client_->async_recv(async_mqtt::filter::match, { packet_t }, asio::use_awaitable);
    }
    return mqtt_client_->async_recv(async_mqtt::filter::match, { packet_t }, asio::use_awaitable);
  }

  template <typename... args_t>
  auto send(args_t&&... args) {
    if (mqtts_client_) {
      return mqtts_client_->async_send(std::forward<decltype(args)>(args)...);
    }
    return mqtt_client_->async_send(std::forward<decltype(args)>(args)...);
  }

  template <typename... args_t>
  auto close(args_t&&... args) {
    if (mqtts_client_) {
      return mqtts_client_->async_close(std::forward<decltype(args)>(args)...);
    }
    return mqtt_client_->async_close(std::forward<decltype(args)>(args)...);
  }

  auto acquire_unique_packet_id() {
    if (mqtts_client_) {
      return mqtts_client_->acquire_unique_packet_id();
    }
    return mqtt_client_->acquire_unique_packet_id();
  }

  auto async_connect(asio::ip::tcp::resolver::results_type resolved_ip) -> asio::awaitable<void> {
    if (mqtts_client_) {
      co_return co_await async_connect_loop(mqtts_client_, resolved_ip);
    }
    co_return co_await async_connect_loop(mqtt_client_, resolved_ip);
  }

  template <typename client_t>
  auto async_connect_loop(client_t& client, asio::ip::tcp::resolver::results_type resolved_ip) -> asio::awaitable<void> {
    while (true) {
      auto res = co_await (asio::async_connect(client->lowest_layer(), resolved_ip, asio::use_awaitable) ||
                           asio::steady_timer{ io_ctx_, std::chrono::seconds{ 1 } }.async_wait(asio::use_awaitable));
      if (res.index() == 0) {
        co_return;
      }
    }
  }

  auto set_sni_hostname(std::string_view address) -> void {
    if (mqtts_client_) {
      if (!SSL_set_tlsext_host_name(mqtts_client_->next_layer().native_handle(), address.data())) {
        const boost::system::error_code error{ static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category() };
        throw boost::system::system_error{ error };
      }
    }
  }

  auto async_handshake() -> asio::awaitable<void> {
    if (mqtts_client_) {
      co_await mqtts_client_->next_layer().async_handshake(async_mqtt::tls::stream_base::client, asio::use_awaitable);
    }
    co_return;
  }

private:
  asio::io_context& io_ctx_;
  async_mqtt::tls::context tls_ctx_{ async_mqtt::tls::context::tlsv12 };
  std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>> mqtt_client_;
  std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>> mqtts_client_;
};

}  // namespace tfc::mqtt
