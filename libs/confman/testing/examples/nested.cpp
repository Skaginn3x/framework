#include <cstdint>
#include <string>
#include <chrono>

#include <fmt/core.h>
#include <fmt/chrono.h>
#include <boost/asio.hpp>
#include <units/quantity.h>
#include <units/isq/si/electric_current.h>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

struct third_level {
  units::aliases::isq::si::electric_current::dA<uint16_t> amper{};
  struct glaze {
    using type = third_level;
    static constexpr auto value{ glz::object("amper", &type::amper, "amper description") };
    static constexpr auto name{ "third_level" };
  };
};

struct second_level {
  std::string a{};
  tfc::confman::observable<std::chrono::nanoseconds> sec{};
  third_level third_lvl{};
  struct glaze {
    using type = second_level;
    static constexpr auto value{ glz::object("a", &type::a, "A description",
                                             "sec", &type::sec, "sec description",
                                             "third_level", &type::third_lvl, "third_lvl description") };
    static constexpr auto name{ "second_level" };
  };
};

struct first_level {
  int a{};
  units::aliases::isq::si::electric_current::dA<uint16_t> amper{};
  second_level second_lvl{};
  struct glaze {
    using type = first_level;
    static constexpr auto value{ glz::object("a_int", &type::a, "A int description",
                                             "amper", &type::amper, "amper description",
                                             "second_lvl", &type::second_lvl, "second_lvl description") };
    static constexpr auto name{ "first_level" };
  };
};

int main() {
  asio::io_context ctx{};

  tfc::confman::config<first_level> const config{ ctx, "key" };
  config->second_lvl.sec.observe([](auto new_value, auto old_value){
    fmt::print("new value: {}, old value: {}\n", new_value, old_value);
  });

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  ctx.run();
  return 0;
}
