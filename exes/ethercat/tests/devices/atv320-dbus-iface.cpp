#include <string_view>

#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/ec/devices/schneider/atv320/dbus-iface.hpp>
#include <tfc/mocks/ipc.hpp>
#include <tfc/progbase.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/stubs/confman.hpp>

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
using mp_units::si::nano;
using mp_units::si::unit_symbols::dHz;
using mp_units::si::unit_symbols::mm;

using tfc::ec::devices::schneider::atv320::controller;
using tfc::ec::devices::schneider::atv320::hmis_e;
using tfc::ec::devices::schneider::atv320::input_t;
using tfc::ec::devices::schneider::atv320::lft_e;
using tfc::ec::devices::schneider::atv320::micrometre_t;
using tfc::ec::devices::schneider::atv320::speedratio_t;
using tfc::motor::errors::err_enum;
using bool_slot_t = tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&>;
using bool_signal_t = tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&>;
using positioner_t = tfc::motor::positioner::positioner<
  tfc::ipc_ruler::ipc_manager_client_mock, metre, tfc::confman::stub_config, bool_slot_t>;
using home_travel_t = tfc::confman::observable<std::optional<positioner_t::absolute_position_t>>;

auto get_good_status_stopped() -> input_t {
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

auto get_good_status_running() -> input_t {
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
  asio::io_context ctx{ asio::io_context() };
  std::shared_ptr<sdbusplus::asio::connection> dbus_connection{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  std::uint16_t slave_id{ 0 };
  tfc::ipc_ruler::ipc_manager_client_mock manager{ dbus_connection };
  controller<tfc::ipc_ruler::ipc_manager_client_mock, tfc::confman::stub_config, bool_slot_t> ctrl{
    dbus_connection, manager, slave_id };
  std::array<bool, 10> ran{};
};

auto main(int, char const* const* argv) -> int {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());

  // Good path tests
  "run_at_speedratio terminated by stop"_test = [&] {
    instance inst;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run_at_speedratio(ratio, [&inst](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.stop([](const std::error_code& stop_error) { expect(!stop_error); });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(5ms);
    expect(inst.ran[0]);
  };

  "run_at_speedratio terminated by quick_stop"_test = [&] {
    instance inst;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run_at_speedratio(ratio, [&inst](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([](const std::error_code& stop_error) { expect(!stop_error); });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(5ms);
    expect(inst.ran[0]);
  };

  "convey micrometre"_test = [&] {
    instance inst;
    inst.ctrl.convey(100 * percent, 1000 * micrometre_t::reference, [&inst](std::error_code err, const micrometre_t moved) {
      expect(!err);
      expect(moved == 1000 * micrometre_t::reference);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);
    inst.ctx.run();
    expect(inst.ran[0]);
  };

  "move with no reference"_test = [&] {
    instance inst;
    inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                   [&inst](err_enum err, const micrometre_t moved) {
                     expect(err == err_enum::motor_missing_home_reference);
                     expect(moved == 0 * micrometre_t::reference);
                     inst.ran[0] = true;
                     inst.ctx.stop();
                   });
    inst.ctx.run();
    expect(inst.ran[0]);
  };
  "move with reference"_test = [&] {
    instance inst;
    // Set current as reference
    tfc::confman::stub_config<positioner_t::config_t, tfc::confman::file_storage<positioner_t::config_t>,
                              tfc::confman::detail::config_dbus_client>& config = inst.ctrl.positioner().config_ref();
    config.access().needs_homing_after = home_travel_t{ 1 * mm };
    config.access().mode = tfc::motor::positioner::encoder_config<nano<metre>>{};
    // Writing to the homing travel speed creates the ipc-slot that accepts the homing sensor input.
    config.access().homing_travel_speed = 1 * speedratio_t::reference;

    inst.ctrl.positioner().home();
    auto sig = bool_signal_t(inst.ctx, inst.manager, "homing_sensor");
    expect(inst.manager.slots_.size() == 1);
    expect(inst.manager.signals_.size() == 1);
    inst.manager.connect("test_atv320_dbus_iface.def.bool.homing_sensor_atv320_0",
                         "test_atv320_dbus_iface.def.bool.homing_sensor", [](const std::error_code&) {
                         });

    sig.async_send(true, [&](const std::error_code& err, const std::size_t) {
      expect(!err) << err;
      expect(inst.ctrl.positioner().homing_enabled());
      inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                     [&inst](err_enum err, const micrometre_t moved) {
                       expect(err == err_enum::success);
                       expect(moved == 1000 * micrometre_t::reference);
                       inst.ran[0] = true;
                       inst.ctx.stop();
                     });
      inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);
    });

    inst.ctx.run();
    expect(inst.ran[0]);
  };

  "test stop impl"_test = [&] {
    instance inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
    });
    expect(inst.ran[1]);
  };

  "test quick_stop impl"_test = [&] {
    instance inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
    });
    expect(inst.ran[1]);
  };

  // Interupted operations
  "convey micrometre interupted by stop"_test = [&] {
    instance inst;
    inst.ctrl.convey(100 * percent, 1000 * micrometre_t::reference, [&inst](std::error_code err, const micrometre_t moved) {
      expect(err == std::errc::operation_canceled);
      expect(moved == 800 * micrometre_t::reference);
      inst.ran[0] = true;
    });
    inst.ctrl.positioner().increment_position(800 * micrometre_t::reference);
    inst.ctrl.update_status(get_good_status_running());
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run();
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };

  "convey micrometre interupted by quick_stop"_test = [&] {
    instance inst;
    inst.ctrl.convey(100 * percent, 1000 * micrometre_t::reference, [&inst](std::error_code err, const micrometre_t moved) {
      expect(err == std::errc::operation_canceled);
      expect(moved == 800 * micrometre_t::reference);
      inst.ran[0] = true;
    });
    inst.ctrl.positioner().increment_position(800 * micrometre_t::reference);
    inst.ctrl.update_status(get_good_status_running());
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run();
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };

  "run cancelled"_test = [] {
    instance inst;
    inst.ctrl.run_at_speedratio(100 * percent, [&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "stop cancelled"_test = [] {
    instance inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "quick stop cancelled"_test = [] {
    instance inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "convey micrometre cancelled"_test = [] {
    instance inst;
    inst.ctrl.convey(100 * percent, 100 * micrometre_t::reference, [&inst](std::error_code err, micrometre_t) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "convey micrometre cancelled while stopping"_test = [] {
    instance inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.convey(100 * percent, 100 * micrometre_t::reference, [&inst](std::error_code err, micrometre_t) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.positioner().increment_position(100 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  return EXIT_SUCCESS;
}
