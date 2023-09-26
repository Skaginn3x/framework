#include <chrono>
#include <cstdint>
#include <string>

#include <units/isq/si/electric_current.h>
#include <units/quantity.h>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

struct simple_config {
  int a{};
  std::string b{};
  tfc::confman::observable<bool> c{};
  std::vector<int> d{};
  std::chrono::nanoseconds sec{};
  units::aliases::isq::si::electric_current::dA<uint16_t> amper{};
  struct glaze {
    using type = simple_config;
    static constexpr auto value{ glz::object("a",
                                             &type::a,
                                             "A description",
                                             "b",
                                             &type::b,
                                             "c",
                                             &type::c,
                                             "C description",
                                             "d",
                                             &type::d,
                                             "D description",
                                             "sec",
                                             &type::sec,
                                             "Sec description",
                                             "amper",
                                             &type::amper,
                                             "Amper description") };
    static constexpr auto name{ "simple_config" };
  };
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::confman::config<simple_config> const config{ ctx, "key" };
  config->c.observe(
      [](bool new_value, bool old_value) { fmt::print("new value: {}, old value: {}\n", new_value, old_value); });

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  ctx.run();
  return 0;
}
