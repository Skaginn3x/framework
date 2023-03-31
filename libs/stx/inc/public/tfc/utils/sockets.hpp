#pragma once

#include <string>
#include <string_view>

#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/stx/to_string_view.hpp>

namespace tfc::utils::sockets {

using std::string_view_literals::operator""sv;

static constexpr auto file_path{ "/run/tfc/"sv };
namespace zmq {
inline constexpr auto file_prefix{ "ipc://"sv };
inline constexpr auto tcp_prefix{ "tcp://"sv };
inline constexpr auto udp_prefix{ "udp://"sv };
}  // namespace zmq
inline constexpr auto endpoint_port_delimiter{ ":"sv };

using endpoint_port_t = std::uint16_t;

namespace detail {}  // namespace detail

template <tfc::stx::basic_fixed_string name>
struct ipc_zmq_socket_endpoint {
  static constexpr std::string_view name_v{ name };
  static constexpr std::string_view value{ stx::string_view_join_v<zmq::file_prefix, file_path, name_v> };
};
template <tfc::stx::basic_fixed_string name>
static constexpr auto ipc_zmq_socket_endpoint_v = ipc_zmq_socket_endpoint<name>::value;

template <tfc::stx::basic_fixed_string endpoint_url, endpoint_port_t endpoint_port, std::string_view const& prefix>
struct generic_zmq_socket_endpoint {
  static constexpr std::string_view endpoint_url_v{ endpoint_url };
  static constexpr std::string_view endpoint_port_v{ tfc::stx::to_string_view_v<endpoint_port> };
  static constexpr std::string_view value{
    stx::string_view_join_v<prefix, endpoint_url_v, endpoint_port_delimiter, endpoint_port_v>
  };
};
template <tfc::stx::basic_fixed_string endpoint_url, endpoint_port_t endpoint_port>
static constexpr auto tcp_zmq_socket_endpoint_v =
    generic_zmq_socket_endpoint<endpoint_url, endpoint_port, zmq::tcp_prefix>::value;

template <tfc::stx::basic_fixed_string endpoint_url, endpoint_port_t endpoint_port>
static constexpr auto udp_zmq_socket_endpoint_v =
    generic_zmq_socket_endpoint<endpoint_url, endpoint_port, zmq::udp_prefix>::value;

auto ipc_zmq_socket_endpoint_str(std::string_view name) -> std::string {
  constexpr auto prefix{ ipc_zmq_socket_endpoint_v<""> };
  return std::string{ prefix.data(), prefix.size() } + std::string{ name.data(), name.size() };
}

auto tcp_zmq_socket_endpoint_str(std::string_view endpoint_url, endpoint_port_t endpoint_port) -> std::string {
  constexpr auto prefix{ zmq::tcp_prefix };
  return std::string{ prefix.data(), prefix.size() } + std::string{ endpoint_url.data(), endpoint_url.size() } +
         std::string{ endpoint_port_delimiter.data(), endpoint_port_delimiter.size() } + std::to_string(endpoint_port);
}

auto udp_zmq_socket_endpoint_str(std::string_view endpoint_url, endpoint_port_t endpoint_port) -> std::string {
  constexpr auto prefix{ zmq::udp_prefix };
  return std::string{ prefix.data(), prefix.size() } + std::string{ endpoint_url.data(), endpoint_url.size() } +
         std::string{ endpoint_port_delimiter.data(), endpoint_port_delimiter.size() } + std::to_string(endpoint_port);
}

namespace test {
using std::string_view_literals::operator""sv;

static_assert(ipc_zmq_socket_endpoint_v<"foo"> == "ipc:///run/tfc/foo"sv);
static_assert(tcp_zmq_socket_endpoint_v<"foo", 42> == "tcp://foo:42"sv);
static_assert(udp_zmq_socket_endpoint_v<"foo", 42> == "udp://foo:42"sv);
static_assert(tcp_zmq_socket_endpoint_v<"192.168.1.1", 42> == "tcp://192.168.1.1:42"sv);

}  // namespace test

}  // namespace tfc::utils::sockets
