#include <boost/asio/deadline_timer.hpp>
#include <boost/ut.hpp>

#include <tfc/rpc.hpp>
#include <tfc/utils/socket.hpp>

namespace asio = boost::asio;
namespace rpc = tfc::rpc;
namespace ut = boost::ut;
using boost::ut::operator""_test;
using std::string_literals::operator""s;

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

static constexpr auto rpc_endpoint{ tfc::utils::socket::zmq::ipc_endpoint_v<"rpc_test"> };
static constexpr auto notify_endpoint{ tfc::utils::socket::zmq::ipc_endpoint_v<"notify_test"> };

auto main(int, char**) -> int { // NOLINT:bugprone-exception-escape
  static constexpr glz::rpc::detail::basic_fixed_string method_name{ "foo" };
  using server_t = rpc::server<glz::rpc::server<glz::rpc::server_method_t<method_name, std::string, int>>>;
  using client_t = rpc::client<glz::rpc::client<glz::rpc::client_method_t<method_name, std::string, int>>>;

  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
    "rpc happy path"_test = [] {
      asio::io_context ctx{};
      server_t server{ ctx, rpc_endpoint };

      std::string expected_input{ "hello world"s };
      int constexpr expected_output{ 1337 };
      bool called{};

      server.converter().on<method_name>([&expected_input, &called](std::string const& input) noexcept -> int {
        called = true;
        ut::expect(input == expected_input);
        return expected_output;
      });

      client_t client{ ctx, rpc_endpoint };
      client.async_request<method_name>(expected_input, [](glz::expected<int, glz::rpc::error> const& value, auto const&) {
        ut::expect(value.has_value());
        if (value.has_value()) {
          ut::expect(value.value() == expected_output);
        }
      });

      ctx.run_for(std::chrono::milliseconds(1));
      ut::expect(called);
    };

    "notify happy path"_test = [] {
      asio::io_context ctx{};
      server_t server{ ctx, notify_endpoint };

      std::string expected_input{ "hello world"s };
      bool called{};

      server.converter().on<method_name>([&expected_input, &called](std::string const& input) noexcept -> int {
        called = true;
        ut::expect(input == expected_input);
        return 0;
      });

      client_t client{ ctx, notify_endpoint };
      client.async_notify<method_name>(expected_input);

      ctx.run_for(std::chrono::milliseconds(1));
      ut::expect(called);
    };

    "server crash"_test = [] {
      // Too much effort, manual testing with json rpc example
    };
  };

  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
