#include <chrono>
#include <cstdint>
#include <string>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <units/isq/si/electric_current.h>
#include <units/quantity.h>
#include <boost/asio.hpp>

#include <tfc/progbase.hpp>
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
  constexpr auto operator==(option_1 const& rhs) const noexcept -> bool = default;
};

struct option_2 {
  std::string a{};
  tfc::confman::observable<std::chrono::nanoseconds> sec{};
  struct glaze {
    using type = option_2;
    static constexpr auto value{ glz::object("a", &type::a, "A description", "sec", &type::sec, "sec description") };
    static constexpr std::string_view name{ "option_2" };
  };
  constexpr auto operator==(option_2 const& rhs) const noexcept -> bool = default;
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
  constexpr auto operator==(with_variant const& rhs) const noexcept -> bool = default;
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::confman::config<tfc::confman::observable<std::vector<with_variant>>> const config{ ctx, "key" };
  config->observe([](auto const& new_value, auto const& old_value) {
    fmt::print("new value:\n{}\n\n\nold value:\n{}\n", glz::write_json(new_value), glz::write_json(old_value));
  });

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  ctx.run();
  return 0;
}
