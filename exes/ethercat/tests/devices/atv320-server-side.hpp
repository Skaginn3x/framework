#pragma once

#include <string_view>

#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/ec/devices/schneider/atv320/dbus-iface.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/mocks/ipc.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/testing/asio_clock.hpp>

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

[[maybe_unused]] static auto get_good_status_stopped() -> input_t {
  return input_t{
    .status_word =
    tfc::ec::cia_402::status_word{
      .state_ready_to_switch_on = 1, .state_switched_on = 1, .voltage_enabled = 1, .state_quick_stop = 1 },
    .frequency = 0 * dHz,
    .current = 0,
    .digital_inputs = 0x0000,
    .last_error = lft_e::no_fault,
    .drive_state = hmis_e::rdy,
  };
}

[[maybe_unused]] static auto get_bad_status_communication_failure(bool running = false) -> input_t {
  return input_t{
    .status_word = tfc::ec::cia_402::status_word{ .state_fault = true },
    .frequency = running ? 100 * dHz : 0 * dHz,
    .current = 0,
    .digital_inputs = 0x0000,
    .last_error = lft_e::cnf,
    .drive_state = hmis_e::fault,
  };
}

[[maybe_unused]] static auto get_bad_status_missing_phase(bool running = false) -> input_t {
  return input_t{
    .status_word = tfc::ec::cia_402::status_word{ .state_fault = true },
    .frequency = running ? 100 * dHz : 0 * dHz,
    .current = 0,
    .digital_inputs = 0x0000,
    .last_error = lft_e::opf1,
    .drive_state = hmis_e::fault,
  };
}

[[maybe_unused]] static auto get_good_status_running() -> input_t {
  return input_t{
    .status_word = tfc::ec::cia_402::status_word{ .state_ready_to_switch_on = 1,
                                                  .state_switched_on = 1,
                                                  .state_operation_enabled = 1,
                                                  .voltage_enabled = 1,
                                                  .state_quick_stop = 1 },
    .frequency = 50 * dHz,
    .current = 10,
    .digital_inputs = 0x0000,
    .last_error = lft_e::no_fault,
    .drive_state = hmis_e::run,
  };
}

struct instance {
  instance() {
    dbus_connection->request_name(tfc::dbus::make_dbus_name(tfc::motor::dbus::detail::service).c_str());
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
  std::array<bool, 10> ran{};
  bool_signal_t sig{ ctx, manager, "homing_sensor" };

  void populate_homing_sensor(micrometre_t displacement = 1 * micrometre_t::reference, bool do_homing = true) {
    tfc::confman::stub_config<positioner_t::config_t, tfc::confman::file_storage<positioner_t::config_t>,
                              tfc::confman::detail::config_dbus_client>& config = ctrl.positioner().config_ref();
    config.access().needs_homing_after = home_travel_t{ 1000000 * mm }; // 1 km
    auto mode = tfc::motor::positioner::encoder_config<nano<metre>>{};
    mode.displacement_per_increment = displacement;
    config.access().mode = mode;
    // Writing to the homing travel speed creates the ipc-slot that accepts the homing sensor input.
    config.access().homing_travel_speed = 1 * speedratio_t::reference;

    if (do_homing) {
      ctrl.positioner().home();
    }
    ctx.run_for(1ms);
    assert(manager.slots_.size() > 1);
    assert(manager.signals_.size() == 1);
    manager.connect(fmt::format("{}.{}.bool.homing_sensor_atv320_{}", tfc::base::get_exe_name(), tfc::base::get_proc_name(), slave_id),
                    fmt::format("{}.{}.bool.homing_sensor", tfc::base::get_exe_name(), tfc::base::get_proc_name()), [&](const std::error_code&) {
                    });
    ctx.run_for(5ms);
    sig.send(true);
    ctx.run_for(5ms);
    assert(ctrl.positioner().homing_enabled());
  }
};
