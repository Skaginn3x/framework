#pragma once

/// \file zmq_socket_endpoint.hpp
/// \brief Defines utility functions and types to generate ZeroMQ socket endpoints

#include <string>
#include <string_view>

#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/stx/to_string_view.hpp>

namespace tfc::utils::socket {

using std::string_view_literals::operator""sv;
using endpoint_port_t = std::uint16_t;

static constexpr auto file_path{ "/var/run/tfc/"sv };
inline constexpr auto endpoint_port_delimiter{ ":"sv };

namespace zmq {

inline constexpr auto file_prefix{ "ipc://"sv };
inline constexpr auto tcp_prefix{ "tcp://"sv };
inline constexpr auto udp_prefix{ "udp://"sv };

/// \brief Type representing a ZeroMQ IPC socket endpoint
/// \tparam name The name of the IPC socket endpoint
/// \note Constructs IPC socket endpoint at compile time
template <tfc::stx::basic_fixed_string name>
struct ipc_endpoint {
  static constexpr std::string_view name_v{ name };
  static constexpr std::string_view value{ stx::string_view_join_v<zmq::file_prefix, file_path, name_v> };
};

/// \brief Utility to extract the value of the IPC socket endpoint
/// \tparam name The name of the IPC socket endpoint
/// \var fully qualified string_view of ZeroMQ IPC socket
template <tfc::stx::basic_fixed_string name>
static constexpr auto ipc_endpoint_v = ipc_endpoint<name>::value;

/// \brief Type representing a generic ZeroMQ socket endpoint
/// \tparam endpoint_url The URL of the socket endpoint
/// \tparam endpoint_port The port of the socket endpoint
/// \tparam prefix The prefix of the socket endpoint URL
template <tfc::stx::basic_fixed_string endpoint_url, endpoint_port_t endpoint_port, std::string_view const& prefix>
struct generic_endpoint {
  static constexpr std::string_view endpoint_url_v{ endpoint_url };
  static constexpr std::string_view endpoint_port_v{ tfc::stx::to_string_view_v<endpoint_port> };
  static constexpr std::string_view value{
    stx::string_view_join_v<prefix, endpoint_url_v, endpoint_port_delimiter, endpoint_port_v>
  };
};

/// \brief Utility to extract the value of the TCP socket endpoint
/// \tparam endpoint_url The URL of the socket endpoint
/// \tparam endpoint_port The port of the socket endpoint
/// \var fully qualified string_view of ZeroMQ TCP socket
template <tfc::stx::basic_fixed_string endpoint_url, endpoint_port_t endpoint_port>
static constexpr auto tcp_endpoint_v = generic_endpoint<endpoint_url, endpoint_port, zmq::tcp_prefix>::value;

/// \brief Utility to extract the value of the UDP socket endpoint
/// \tparam endpoint_url The URL of the socket endpoint
/// \tparam endpoint_port The port of the socket endpoint
/// \var fully qualified string_view of ZeroMQ UDP socket
template <tfc::stx::basic_fixed_string endpoint_url, endpoint_port_t endpoint_port>
static constexpr auto udp_endpoint_v = generic_endpoint<endpoint_url, endpoint_port, zmq::udp_prefix>::value;

/// \brief Runtime utility to create name of IPC socket endpoint
/// \param name The name of the IPC socket endpoint
/// \return fully qualified string of ZeroMQ IPC socket
[[maybe_unused]] static auto ipc_endpoint_str(std::string_view name) -> std::string {
  constexpr auto prefix{ ipc_endpoint_v<""> };
  return std::string{ prefix.data(), prefix.size() } + std::string{ name.data(), name.size() };
}

/// \brief Runtime utility to create name of TCP socket endpoint
/// \param endpoint_url The URL of the socket endpoint
/// \param endpoint_port The port of the socket endpoint
/// \return fully qualified string of ZeroMQ TCP socket
[[maybe_unused]] static auto tcp_endpoint_str(std::string_view endpoint_url, endpoint_port_t endpoint_port) -> std::string {
  constexpr auto prefix{ zmq::tcp_prefix };
  return std::string{ prefix.data(), prefix.size() } + std::string{ endpoint_url.data(), endpoint_url.size() } +
         std::string{ endpoint_port_delimiter.data(), endpoint_port_delimiter.size() } + std::to_string(endpoint_port);
}

/// \brief Runtime utility to create name of UDP socket endpoint
/// \param endpoint_url The URL of the socket endpoint
/// \param endpoint_port The port of the socket endpoint
/// \return fully qualified string of ZeroMQ UDP socket
[[maybe_unused]] static auto udp_endpoint_str(std::string_view endpoint_url, endpoint_port_t endpoint_port) -> std::string {
  constexpr auto prefix{ zmq::udp_prefix };
  return std::string{ prefix.data(), prefix.size() } + std::string{ endpoint_url.data(), endpoint_url.size() } +
         std::string{ endpoint_port_delimiter.data(), endpoint_port_delimiter.size() } + std::to_string(endpoint_port);
}

namespace test {
using std::string_view_literals::operator""sv;

static_assert(ipc_endpoint_v<"foo"> == "ipc:///tmp/foo"sv);
static_assert(tcp_endpoint_v<"foo", 42> == "tcp://foo:42"sv);
static_assert(udp_endpoint_v<"foo", 42> == "udp://foo:42"sv);
static_assert(tcp_endpoint_v<"192.168.1.1", 42> == "tcp://192.168.1.1:42"sv);

}  // namespace test

}  // namespace zmq

}  // namespace tfc::utils::socket
