#pragma once

#include <string_view>

namespace tfc::confman::detail {

constexpr std::string_view rpc_socket = "/tmp/confman.rpc.sock";
constexpr std::string_view notify_socket = "/tmp/confman.notify.sock";

struct alive {
  glz::raw_json_view defaults{};
  std::string identity{};
  struct glaze {
    static auto constexpr value{ glz::object("id", &alive::identity, "defaults", &alive::defaults) };
  };
};

struct alive_result {
  glz::raw_json config{};
  struct glaze {
    static auto constexpr value{ glz::object("config", &alive_result::config) };
  };
};

}  // namespace tfc::confman::detail
