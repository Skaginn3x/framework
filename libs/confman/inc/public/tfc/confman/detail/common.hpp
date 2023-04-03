#pragma once

#include <string_view>

#include <tfc/progbase.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/utils/socket.hpp>

#include <glaze/glaze.hpp>

namespace tfc::confman::detail {

// If changed remind to update systemd socket unit
inline constexpr std::string_view rpc_socket_path{ utils::socket::zmq::ipc_endpoint_v<"confman.rpc.sock"> };
inline constexpr std::string_view notify_socket_path{ utils::socket::zmq::ipc_endpoint_v<"confman.notify.sock"> };

namespace method {

inline constexpr tfc::stx::basic_fixed_string alive_tag{ "alive" };

struct alive {
  glz::raw_json_view schema{};
  glz::raw_json_view defaults{};  // todo this should not be necessary, we should be able to inject defaults to schema
  std::string identity{};
  struct glaze {
    static auto constexpr value{
      glz::object("id", &alive::identity, "schema", &alive::schema, "defaults", &alive::defaults)
    };
  };
};

struct alive_result {
  glz::raw_json_view config{};
  struct glaze {
    static auto constexpr value{ glz::object("config", &alive_result::config) };
  };
};

}  // namespace method

}  // namespace tfc::confman::detail
