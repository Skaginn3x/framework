#pragma once

#include <algorithm>
#include <any>
#include <chrono>
#include <concepts>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <openssl/ssl.h>
#include <sparkplug_b/sparkplug_b.pb.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <tfc/dbus/string_maker.hpp>

namespace asio = boost::asio;

template <class config_type>
class mqtt_client_wrapper_ {
public:
  mqtt_client_wrapper_(asio::io_context& io_ctx, config_type& config)
      : io_ctx_(io_ctx), config_(std::move(config)) {

    if (config_.value().ssl_active == tfc::mqtt::ssl::yes) {
      auto d =
          async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>{ async_mqtt::protocol_version::v5,
                                                                                       io_ctx_.get_executor(), tls_ctx_ };
    } else {
      auto k =
          async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>{ async_mqtt::protocol_version::v5,
                                                                                      io_ctx_.get_executor() };
    }

  }

  auto get_strand() -> decltype(auto) {
    return std::visit(
        [](auto&& arg) -> decltype(auto) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (!std::is_same_v<T, std::monostate>) {
            return arg.strand();
          }
          throw std::invalid_argument("mqtt_client_ is in uninitialized (monostate) state");
        },
        mqtt_client_);
  }

private:
  asio::io_context& io_ctx_;
  config_type config_;
  async_mqtt::tls::context tls_ctx_{ async_mqtt::tls::context::tlsv12 };
  std::variant<std::monostate,
               async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtt>,
               async_mqtt::endpoint<async_mqtt::role::client, async_mqtt::protocol::mqtts>>
      mqtt_client_ = std::monostate{};
};
