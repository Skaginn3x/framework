//
// Created by omar on 3/2/23.
//
#include "tfc/rpc.hpp"
#include <glaze/glaze.hpp>

auto main() -> int {
  tfc::rpc::server <
  tfc::rpc::method_t<"get_config", get_config_request, get_config_return>,
  tfc::rpc::method_t<"set_config", set_config_request, set_config_return>
  > server;

  server.on<"get_config">([](get_config_request const& request) -> get_config_return {
    fmt::print("Got get_config request:\nexe_name:{}\nid:{}\nkey:{}\n\n", request.executable_name, request.executable_id, request.config_key);
    return get_config_return{.json="this is my response"};
  });

  server.on<"set_config">([](set_config_request const& request) -> set_config_return {
    fmt::print("Got set_config request:\nexe_name:{}\nid:{}\nkey:{}\nvalue:{}\n\n", request.executable_name, request.executable_id, request.config_key, request.value);
    return set_config_return{.succeeded=false, .error="can't succeed straight away"};
  });

  server.call("get_config", R"({"exe_name": "foo", "id": "bar", "key": "beers" })");
  server.call("set_config", R"({"exe_name": "foo", "id": "bar", "key": "beers", "value": "42" })");

  return 0;
}
