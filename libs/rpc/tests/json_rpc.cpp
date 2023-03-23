#include <boost/asio/deadline_timer.hpp>
#include <boost/ut.hpp>
#include <tfc/rpc.hpp>
namespace asio = boost::asio;
namespace rpc = tfc::rpc;
namespace ut = boost::ut;
using boost::ut::operator""_test;
using std::string_literals::operator""s;
using std::string_view_literals::operator""sv;

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

auto main(int, char**) -> int {
  static constexpr glz::rpc::detail::basic_fixed_string method_name{ "foo" };
  using server_t = rpc::server<glz::rpc::server<glz::rpc::server_method_t<method_name, std::string, int>>>;
  using client_t = rpc::client<glz::rpc::client<glz::rpc::client_method_t<method_name, std::string, int>>>;

  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
    "rpc happy path"_test = [] {
      asio::io_context ctx{};
      server_t server{ ctx, "/tmp/fthis" };

      std::string expected_input{ "hello world"s };
      int constexpr expected_output{ 1337 };
      bool called{};

      server.converter().on<method_name>([expected_input, &called](std::string const& input) -> int {
        called = true;
        ut::expect(input == expected_input);
        return expected_output;
      });

      client_t client{ ctx, "/tmp/fthis" };
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
      server_t server{ ctx, "/tmp/notify_test" };

      std::string expected_input{ "hello world"s };
      bool called{};

      server.converter().on<method_name>([expected_input, &called](std::string const& input) -> int {
        called = true;
        ut::expect(input == expected_input);
        return 0;
      });

      client_t client{ ctx, "/tmp/notify_test" };
      client.async_notify<method_name>(expected_input);

      ctx.run_for(std::chrono::milliseconds(1));
      ut::expect(called);
    };

    "server crash"_test = [] {
      // Too much effort, manual testing with json rpc example
    };
  };

  return boost::ut::cfg<>.run({ .report_errors = true });
}
