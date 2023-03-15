#include <glaze/glaze.hpp>
#include "tfc/rpc.hpp"

struct get_config_request {
  std::string executable_name{};
  std::string executable_id{};
  std::string config_key{};
  struct glaze {
    using T = get_config_request;
    static constexpr auto value =
        glz::object("exe_name", &T::executable_name, "id", &T::executable_id, "key", &T::config_key);
  };
};

struct get_config_return {
  std::string json{};
  struct glaze {
    using T = get_config_return;
    static constexpr auto value = glz::object("json", &T::json);
  };
};

struct set_config_request {
  std::string executable_name{};
  std::string executable_id{};
  std::string config_key{};
  std::string value{};
  struct glaze {
    using T = set_config_request;
    static constexpr auto value =
        glz::object("exe_name", &T::executable_name, "id", &T::executable_id, "key", &T::config_key, "value", &T::value);
  };
};

struct set_config_return {
  bool succeeded{false};
  std::string error{};
  struct glaze {
    using T = set_config_return;
    static constexpr auto value = glz::object("succeeded", &T::succeeded, "error", &T::error);
  };
};

auto main() -> int {
  asio::io_context ctx{};

  tfc::rpc::server<glz::rpc::server<glz::rpc::server_method_t<"get_config", get_config_request, get_config_return>,
                                    glz::rpc::server_method_t<"set_config", set_config_request, set_config_return> > >
      server(ctx, "my_rpc_server");


  //  server.on<"get_config">([](get_config_request const& request) -> get_config_return {
  //    fmt::print("Got get_config request:\nexe_name:{}\nid:{}\nkey:{}\n\n", request.executable_name, request.executable_id,
  //    request.config_key); return get_config_return{.json="this is my response"};
  //  });
  //
  //  server.on<"set_config">([](set_config_request const& request) -> set_config_return {
  //    fmt::print("Got set_config request:\nexe_name:{}\nid:{}\nkey:{}\nvalue:{}\n\n", request.executable_name,
  //    request.executable_id, request.config_key, request.value); return set_config_return{.succeeded=false, .error="can't
  //    succeed straight away"};
  //  });
  //
  //  server.call("get_config", R"({"exe_name": "foo", "id": "bar", "key": "beers" })");
  //  server.call("set_config", R"({"exe_name": "foo", "id": "bar", "key": "beers", "value": "42" })");

  return 0;
}
