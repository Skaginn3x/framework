#include <fmt/core.h>
#include <boost/asio.hpp>
#include <type_traits>
#include <algorithm>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>

namespace asio = boost::asio;

struct object_in_array {
  int a{};
  struct glaze {
    using type = object_in_array;
    static constexpr auto value{
      glz::object("a_int", &type::a, "A int description")
    };
    static constexpr auto name{ "object_in_array" };
  };
};

int main() {
  asio::io_context ctx{};

  tfc::confman::config<std::vector<object_in_array>> const config{ ctx, "key" };

  std::vector a {4,3,2,1};
  std::sort(a.begin(), a.end());
  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  ctx.run();
  return 0;
}
