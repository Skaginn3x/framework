#include <type_traits>
#include <mp-units/systems/si/si.h>
#include <mp-units/systems/isq/isq.h>

#include <boost/ut.hpp>
#include <tfc/progbase.hpp>
#include <tfc/motors/mock.hpp>
#include <boost/ut.hpp>

using namespace mp_units::si::unit_symbols;
using namespace tfc::motor::impl;

using namespace mp_units::si::unit_symbols;  // NOLINT(*-build-using-namespace)
namespace asio = boost::asio;

namespace ut = boost::ut;
using ut::operator""_test;
using ut::expect;
using ut::operator>>;
using ut::fatal;
using ut::operator|;

int main(int argc, char** argv) {
  using boost::ut::operator""_test;
  using boost::ut::expect;

  tfc::base::init(argc, argv);

  "run for some time"_test = []() {
    asio::io_context ctx;
    tfc::motor::types::virtual_motor motor{ ctx, tfc::motor::types::virtual_motor::config_t{} };
    boost::ut::expect(!motor.is_running());

    motor.convey(1 * ms, [&](std::error_code err) {
      boost::ut::expect(!err);
      boost::ut::expect(!motor.is_running());
    });


    ctx.run();
  };
}
