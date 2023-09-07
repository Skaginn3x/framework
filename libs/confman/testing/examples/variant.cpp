#include <chrono>
#include <cstdint>
#include <string>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <units/isq/si/electric_current.h>
#include <units/quantity.h>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

struct option_1 {
  units::aliases::isq::si::electric_current::dA<uint16_t> amper{};
  struct glaze {
    using type = option_1;
    static constexpr auto value{ glz::object("amper", &type::amper, "amper description") };
    static constexpr std::string_view name{ "option_1" };
  };
};

struct option_2 {
  std::string a{};
  tfc::confman::observable<std::chrono::nanoseconds> sec{};
  struct glaze {
    using type = option_2;
    static constexpr auto value{ glz::object("a", &type::a, "A description", "sec", &type::sec, "sec description") };
    static constexpr std::string_view name{ "option_2" };
  };
};

struct with_variant {
  int a{};
  std::variant<std::monostate, option_1, option_2> variant{};
  struct glaze {
    using type = with_variant;
    static constexpr auto value{
      glz::object("a_int", &type::a, "A int description", "variant", &type::variant, "variant description")
    };
    static constexpr std::string_view name{ "with_variant" };
  };
};

int main() {
  asio::io_context ctx{};

  tfc::confman::config<with_variant> const config{ ctx, "key" };

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  ctx.run();
  return 0;
}
