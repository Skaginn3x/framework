#include <chrono>
#include <cstdint>
#include <string>

#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;
using velocity = mp_units::quantity<mp_units::si::metre / mp_units::si::second, int>;

struct simple_config {
  velocity velo{ 1 * mp_units::si::metre / mp_units::si::second };

  struct glaze {
    using type = simple_config;
    static constexpr auto value{ glz::object("velo", &type::velo) };
    static constexpr auto name{ "simple_config" };
  };
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::confman::config<simple_config> const config{ ctx, "key" };

  ctx.run();
  return 0;
}
