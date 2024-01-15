#include <string_view>

#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/ec/devices/schneider/atv320/dbus-iface.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/mocks/ipc.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/testing/asio_clock.hpp>
#include <tfc/motor/atv320motor.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;

using asio::steady_timer;

using std::chrono::operator""ms;
using std::string_view_literals::operator""sv;

using ut::operator""_test;
using ut::operator|;
using ut::expect;

using mp_units::percent;
using mp_units::si::metre;
using mp_units::si::micro;
using mp_units::si::milli;
using mp_units::si::nano;
using mp_units::si::unit_symbols::dHz;
using mp_units::si::unit_symbols::km;
using mp_units::si::unit_symbols::mm;

using tfc::ec::devices::schneider::atv320::controller;
using tfc::ec::devices::schneider::atv320::dbus_iface;
using tfc::motor::types::atv320motor;
using tfc::ec::devices::schneider::atv320::hmis_e;
using tfc::ec::devices::schneider::atv320::input_t;
using tfc::ec::devices::schneider::atv320::lft_e;
using tfc::ec::devices::schneider::atv320::micrometre_t;
using tfc::ec::devices::schneider::atv320::speedratio_t;
using tfc::motor::errors::err_enum;
using bool_slot_t = tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&>;
using bool_signal_t = tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&>;
using positioner_t = tfc::motor::positioner::
positioner<metre, tfc::ipc_ruler::ipc_manager_client_mock&, tfc::confman::stub_config, bool_slot_t>;
using home_travel_t = tfc::confman::observable<std::optional<positioner_t::absolute_position_t>>;

struct instance {
  instance() {
    atv_conf.slave_id = slave_id;
  }

  asio::io_context ctx{ asio::io_context() };
  std::shared_ptr<sdbusplus::asio::connection> dbus_connection{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  std::uint16_t slave_id{ 999 };
  tfc::ipc_ruler::ipc_manager_client_mock manager{ dbus_connection };
  using timer_t = asio::basic_waitable_timer<tfc::testing::clock, tfc::testing::wait_traits>;
  controller<tfc::ipc_ruler::ipc_manager_client_mock, tfc::confman::stub_config, timer_t, bool_slot_t> ctrl{ dbus_connection,
    manager,
    slave_id };
  dbus_iface<tfc::ipc_ruler::ipc_manager_client_mock, tfc::confman::stub_config, timer_t, bool_slot_t> server{
    ctrl, dbus_connection, slave_id };
  atv320motor::config_t atv_conf;
  atv320motor client{ dbus_connection, atv_conf };
  std::array<bool, 10> ran{};
};

auto main(int, char const* const* argv) -> int {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());
  "test ping from client to server"_test = [] {
    instance inst;
    inst.ctx.run_for(1ms);
    expect(inst.client.connected());
  };
}
