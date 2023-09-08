#pragma once
#include <openssl/ssl.h>
#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>

namespace tfc::mqtt::impl {
namespace asio = boost::asio;

class network_manager_ssl {
public:
  static auto connect_socket(auto&& socket, asio::ip::tcp::resolver::results_type resolved_ip) -> asio::awaitable<void> {
    co_await asio::async_connect(std::forward<decltype(socket)>(socket), resolved_ip, asio::use_awaitable);
  }

  // ssl
  static auto set_sni_hostname(auto&& native_handle, std::string const& broker_address) -> void {
    using socket_t = std::remove_cvref_t<decltype(native_handle)>;

    if constexpr (std::is_same_v<socket_t, SSL*>) {
      if (!SSL_set_tlsext_host_name(std::forward<decltype(native_handle)>(native_handle), broker_address.c_str())) {
        const boost::system::error_code error{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
        throw boost::system::system_error{ error };
      }
    }
  }

  static auto async_handshake(auto&& socket) -> asio::awaitable<void> {
    using socket_t = std::remove_cvref_t<decltype(socket)>;
    if constexpr (std::is_same_v<
                      socket_t,
                      boost::asio::ssl::stream<boost::asio::basic_stream_socket<
                          boost::asio::ip::tcp, boost::asio::io_context::basic_executor_type<std::allocator<void>, 0>>>>) {
      co_await std::forward<decltype(socket)>(socket).async_handshake(async_mqtt::tls::stream_base::client,
                                                                      asio::use_awaitable);
    }
    co_return;
  }
};
}  // namespace tfc::mqtt::impl
