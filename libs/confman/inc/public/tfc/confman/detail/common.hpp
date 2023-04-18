#pragma once

#include <string_view>

#include <tfc/ipc/enums.hpp>
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

struct alive {
  glz::raw_json_view schema{};
  glz::raw_json_view defaults{};  // todo this should not be necessary, we should be able to inject defaults to schema
  std::string identity{};
  static constexpr tfc::stx::basic_fixed_string tag{ "alive" };
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

struct get_ipcs {
  ipc::direction_e direction{};
  ipc::type_e type{};
  std::string name_contains{};
  static stx::basic_fixed_string constexpr tag{ "get_ipcs" };
  struct glaze {
    using T = get_ipcs;
    // clang-format off
    static auto constexpr value{ glz::object(
        "direction", &T::direction, "IPC direction, signal or slot",
        "type", &T::type, "IPC type being transmitted",
        "name_contains", &T::name_contains, "Filter by containing string in IPC name"
        ) };
    // clang-format on
    static std::string_view constexpr name{ tag };
  };
};

using get_ipcs_result = std::vector<std::string>;

}  // namespace method

}  // namespace tfc::confman::detail
