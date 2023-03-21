#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <glaze/ext/jsonrpc.hpp>
#include <glaze/glaze.hpp>

#include <tfc/progbase.hpp>
#include <tfc/rpc.hpp>

namespace asio = boost::asio;
namespace bpo = boost::program_options;

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
  bool succeeded{ false };
  std::string error{};
  struct glaze {
    using T = set_config_return;
    static constexpr auto value = glz::object("succeeded", &T::succeeded, "error", &T::error);
  };
};

using server_t =
    tfc::rpc::server<glz::rpc::server<glz::rpc::server_method_t<"get_config", get_config_request, get_config_return>,
                                      glz::rpc::server_method_t<"set_config", set_config_request, set_config_return> > >;
using client_t =
    tfc::rpc::client<glz::rpc::client<glz::rpc::client_method_t<"get_config", get_config_request, get_config_return>,
                                      glz::rpc::client_method_t<"set_config", set_config_request, set_config_return> > >;

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };

  bool make_client{}, make_server{}, make_faulty_server{};

  desc.add_options()("client", bpo::bool_switch(&make_client)->default_value(false), "Make client.")(
      "server", bpo::bool_switch(&make_server)->default_value(false), "Make server.")(
      "faulty-server", bpo::bool_switch(&make_faulty_server)->default_value(false),
      "Make faulty server. FYI, cannot make both faulty and normal server, faulty server throws(crashes) during any request.");
  tfc::base::init(argc, argv, desc);

  asio::io_context ctx{};

  std::shared_ptr<server_t> server{};

  if (make_server) {
    server = std::make_shared<server_t>(ctx, "/tmp/my_rpc_server");
    server->converter().on<"get_config">([](get_config_request const& request) -> get_config_return {
      fmt::print("Got get_config request:\nexe_name:{}\nid:{}\nkey:{}\n\n", request.executable_name, request.executable_id,
                 request.config_key);
      return get_config_return{ .json = "this is my response" };
    });

    server->converter().on<"set_config">([](set_config_request const& request) -> set_config_return {
      fmt::print("Got set_config request:\nexe_name:{}\nid:{}\nkey:{}\nvalue:{}\n\n", request.executable_name,
                 request.executable_id, request.config_key, request.value);
      return set_config_return{ .succeeded = false, .error = "can't succeed straight away" };
    });
  }
  else if(make_faulty_server) {
    server = std::make_shared<server_t>(ctx, "/tmp/my_rpc_server");
    server->converter().on<"get_config">([](get_config_request const& request) -> get_config_return {
      throw std::runtime_error{"get_config Faulty server"};
    });
    server->converter().on<"set_config">([](set_config_request const& request) -> set_config_return {
      throw std::runtime_error{"set_config Faulty server"};
    });
  }

  std::shared_ptr<client_t> client{};

  if (make_client) {
    client = std::make_shared<client_t>(ctx, "/tmp/my_rpc_server");
    asio::deadline_timer a_timer{ ctx };
    a_timer.expires_from_now(boost::posix_time::milliseconds(1));
    a_timer.async_wait([&client](auto) {
      //
      for (int i = 0; i < 1; ++i) {
        client->async_request<"get_config">(
            get_config_request{ .executable_name = "hello world", .executable_id = "bar", .config_key = "foo" },
            [](glz::expected<get_config_return, glz::rpc::error> const& response, glz::rpc::jsonrpc_id_type const&) -> void {
              if (response.has_value()) {
                fmt::print("Got response: {}\n", response->json);
              } else {
                fmt::print("Got error man: {}\n", response.error().get_message());
              }
            });
      }

      client->async_notify<"set_config">(
          set_config_request{ .executable_name = "notify1", .executable_id = "notify1", .config_key = "notify1" });

      client->async_request<"set_config">(
          set_config_request{ .executable_name = "hello world", .executable_id = "bar", .config_key = "foo" },
          [](glz::expected<set_config_return, glz::rpc::error> const& response, glz::rpc::jsonrpc_id_type const&) -> void {
            if (response.has_value()) {
              fmt::print("Got response:{} {}\n", response->error, response->succeeded);
            } else {
              fmt::print("Got error man:{}\n", response.error().get_message());
            }
          });

      client->async_notify<"set_config">(
          set_config_request{ .executable_name = "notify2", .executable_id = "notify2", .config_key = "notify2" });
    });
  }



  ctx.run();

  return 0;
}
