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

template <typename clock_t = std::chrono::steady_clock>
struct instance {
  asio::io_context ctx{ asio::io_context() };
  std::shared_ptr<sdbusplus::asio::connection> dbus_connection{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  std::uint16_t slave_id{ 0 };
  tfc::ipc_ruler::ipc_manager_client_mock manager{ dbus_connection };
  controller<tfc::ipc_ruler::ipc_manager_client_mock, tfc::confman::stub_config, clock_t, bool_slot_t> ctrl{ dbus_connection,
                                                                                                             manager,
                                                                                                             slave_id };
  std::array<bool, 10> ran{};
  bool_signal_t sig{ ctx, manager, "homing_sensor" };

  void populate_homing_sensor(micrometre_t displacement = 1 * micrometre_t::reference) {
    tfc::confman::stub_config<positioner_t::config_t, tfc::confman::file_storage<positioner_t::config_t>,
                              tfc::confman::detail::config_dbus_client>& config = ctrl.positioner().config_ref();
    config.access().needs_homing_after = home_travel_t{ 1000000 * mm };  // 1 km
    auto mode = tfc::motor::positioner::encoder_config<nano<metre>>{};
    mode.displacement_per_increment = displacement;
    config.access().mode = mode;
    // Writing to the homing travel speed creates the ipc-slot that accepts the homing sensor input.
    config.access().homing_travel_speed = 1 * speedratio_t::reference;

    ctrl.positioner().home();
    ctx.run_for(1ms);
    assert(manager.slots_.size() > 1);
    assert(manager.signals_.size() == 1);
    manager.connect("test_atv320_dbus_iface.def.bool.homing_sensor_atv320_0",
                    "test_atv320_dbus_iface.def.bool.homing_sensor", [&](const std::error_code&) {});
    ctx.run_for(5ms);
    sig.send(true);
    ctx.run_for(5ms);
    assert(ctrl.positioner().homing_enabled());
  }
};

auto main(int, char const* const* argv) -> int {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());

  // Good path tests
  "run_at_speedratio terminated by stop"_test = [&] {
    instance<> inst;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, [&inst](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    inst.ctrl.update_status(get_good_status_running());
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    inst.ctrl.stop([](const std::error_code& stop_error) { expect(!stop_error); });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(5ms);
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::stop);
    expect(inst.ran[0]);
  };

  "run_at_speedratio terminated by quick_stop"_test = [&] {
    instance<> inst;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, [&inst](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([](const std::error_code& stop_error) { expect(!stop_error); });
    inst.ctrl.update_status(get_good_status_stopped());
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::quick_stop);
    inst.ctx.run_for(5ms);
    expect(inst.ran[0]);
  };

  "convey micrometre"_test = [&] {
    instance<> inst;
    auto ratio = 100 * percent;
    inst.ctrl.convey(ratio, 1000 * micrometre_t::reference, [&inst](const std::error_code& err, const micrometre_t moved) {
      expect(!err);
      expect(moved == 1000 * micrometre_t::reference);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::quick_stop);
    expect(inst.ran[0]);
  };

  "run time happy path"_test = [&] {
    using tfc::testing::clock;
    clock::set_ticks(clock::time_point{});
    instance<clock> inst;
    auto duration = 1 * milli<mp_units::si::second>;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctrl.run(ratio, duration, [&inst](const std::error_code& err) -> void {
      expect(!err) << err;
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    clock::set_ticks(clock::now() + mp_units::to_chrono_duration(duration));
    inst.ctx.run_for(2ms);
    expect(inst.ran[0]);
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::quick_stop);
  };

  "run time"_test = [&] {
    using tfc::testing::clock;
    instance<clock> inst;
    auto duration = 3 * milli<mp_units::si::second>;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, duration, [&inst](const std::error_code& err) -> void {
      expect(!err) << err;
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    inst.ctrl.update_status(get_good_status_running());
    clock::set_ticks(clock::now() + 1ms);
    inst.ctx.run_for(2ms);
    expect(!inst.ran[0]);
    clock::set_ticks(clock::now() + 2ms);
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    inst.ctx.run_for(2ms);
    expect(!inst.ran[0]);
    inst.ctrl.update_status(get_good_status_stopped());
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::stop);
    inst.ctx.run_for(2ms);
    expect(inst.ran[0]);
  };

  "move with no reference"_test = [&] {
    instance<> inst;
    inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                   [&inst](const std::error_code& err, const micrometre_t moved) {
                     expect(tfc::motor::motor_enum(err) == err_enum::motor_missing_home_reference);
                     expect(moved == 0 * micrometre_t::reference);
                     inst.ran[0] = true;
                     inst.ctx.stop();
                   });
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::stop);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "move with reference"_test = [&] {
    instance<> inst;
    // set current as reference
    inst.populate_homing_sensor();
    auto ratio = 10 * speedratio_t::reference;
    inst.ctrl.move(ratio, 1000 * micrometre_t::reference, [&inst](const std::error_code& err, const micrometre_t moved) {
      expect(!err);
      expect(moved == 1000 * micrometre_t::reference);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);

    inst.ctx.run_for(5ms);
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::quick_stop);
    expect(inst.ran[0]);
  };
  "move to far"_test = [&] {
    instance<> inst;
    // set current as reference
    inst.populate_homing_sensor();
    expect(inst.ctrl.positioner().homing_enabled());
    inst.ctrl.update_status(get_good_status_stopped());
    // Since populate homing_sensor sets the homing_displacement amount to 1km we can move just over 1km
    inst.ctrl.move(10 * speedratio_t::reference, 2 * km, [&inst](const std::error_code& err, const micrometre_t moved) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_missing_home_reference);
      expect(moved == 0 * micrometre_t::reference);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::none);
    inst.ctrl.positioner().increment_position(0 * micrometre_t::reference);
    inst.ctx.run_for(5ms);
    expect(inst.ran[0]);
  };

  "test stop impl interupted by stop"_test = [&] {
    instance<> inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
    });
    expect(inst.ran[1]);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  "test quick_stop impl"_test = [&] {
    instance<> inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
    });
    expect(inst.ran[1]);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  "move home already in homing sensor"_test = [&] {
    instance<> inst;
    // Set current as reference
    inst.populate_homing_sensor();
    inst.ctrl.move_home([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    inst.ctrl.positioner().increment_position(0 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  "move home not in homing sensor"_test = [&] {
    instance<> inst;
    // Set current as reference
    inst.populate_homing_sensor();
    inst.sig.send(false);
    inst.ctrl.move_home([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);
    inst.sig.send(true);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  // Interupted operations
  "run time interupted by stop"_test = [&] {
    using tfc::testing::clock;
    instance<clock> inst;
    auto duration = 100 * milli<mp_units::si::second>;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, duration, [&inst](const std::error_code& err) -> void {
      expect(err == std::errc::operation_canceled) << err;
      inst.ran[0] = true;
    });
    inst.ctrl.update_status(get_good_status_running());
    clock::set_ticks(clock::now() + 80ms);
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };
  "run time interupted by quick_stop"_test = [&] {
    using tfc::testing::clock;
    instance<clock> inst;
    auto duration = 100 * milli<mp_units::si::second>;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, duration, [&inst](const std::error_code& err) -> void {
      expect(err == std::errc::operation_canceled) << err;
      inst.ran[0] = true;
    });
    inst.ctrl.update_status(get_good_status_running());
    clock::set_ticks(clock::now() + 80ms);
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };
  "convey micrometre interupted by stop"_test = [&] {
    instance<> inst;
    inst.ctrl.convey(100 * percent, 1000 * micrometre_t::reference,
                     [&inst](const std::error_code& err, const micrometre_t moved) {
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
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };

  "convey micrometre interupted by quick_stop"_test = [&] {
    instance<> inst;
    inst.ctrl.convey(100 * percent, 1000 * micrometre_t::reference,
                     [&inst](const std::error_code& err, const micrometre_t moved) {
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
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };
  "move cancelled"_test = [] {
    instance<> inst;
    inst.populate_homing_sensor();
    inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                   [&inst](const std::error_code& err, const micrometre_t) {
                     expect(err == std::errc::operation_canceled) << err.message();
                     inst.ran[0] = true;
                   });
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "move cancelled after length reached but not stopped"_test = [] {
    instance<> inst;
    inst.populate_homing_sensor();
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                   [&inst](const std::error_code& err, const micrometre_t) {
                     expect(err == std::errc::operation_canceled) << err.message();
                     inst.ran[0] = true;
                   });
    inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(5ms);
    expect(inst.ran[0]);
  };
  "move interupted by quick_stop"_test = [&] {
    instance<> inst;
    inst.populate_homing_sensor();
    inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                   [&inst](const std::error_code& err, const micrometre_t moved) {
                     expect(err == std::errc::operation_canceled) << err.message();
                     expect(moved == 800 * micrometre_t::reference) << fmt::format("{}", moved);
                     inst.ran[0] = true;
                   });
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.positioner().increment_position(800 * micrometre_t::reference);
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };

  "move interupted by stop"_test = [&] {
    instance<> inst;
    inst.populate_homing_sensor();
    inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                   [&inst](const std::error_code& err, const micrometre_t moved) {
                     expect(err == std::errc::operation_canceled) << err.message();
                     expect(moved == 800 * micrometre_t::reference) << fmt::format("{}", moved);
                     inst.ran[0] = true;
                   });
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.positioner().increment_position(800 * micrometre_t::reference);
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };
  "run cancelled"_test = [] {
    instance<> inst;
    inst.ctrl.run(100 * percent, [&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "run time canceled"_test = [&] {
    using tfc::testing::clock;
    instance<clock> inst;
    auto duration = 100 * milli<mp_units::si::second>;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, duration, [&inst](const std::error_code& err) -> void {
      expect(err == std::errc::operation_canceled) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    inst.ctrl.update_status(get_good_status_running());
    clock::set_ticks(clock::now() + 80ms);
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "stop cancelled"_test = [] {
    instance<> inst;
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
    instance<> inst;
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
    instance<> inst;
    inst.ctrl.convey(100 * percent, 100 * micrometre_t::reference, [&inst](const std::error_code& err, micrometre_t) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.cancel_pending_operation();
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "stoping canceled by convey"_test = [] {
    instance<> inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err.message();
      inst.ran[0] = true;
    });
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.convey(100 * percent, 100 * micrometre_t::reference, [&inst](const std::error_code& err, micrometre_t) {
      expect(tfc::motor::motor_enum(err) == err_enum::success);
      inst.ran[1] = true;
    });
    inst.ctrl.positioner().increment_position(100 * micrometre_t::reference);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };
  "quick_stop canceled by convey"_test = [] {
    instance<> inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::operation_canceled) << err.message();
      inst.ran[0] = true;
    });
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.convey(100 * percent, 100 * micrometre_t::reference, [&inst](const std::error_code& err, micrometre_t) {
      expect(tfc::motor::motor_enum(err) == err_enum::success);
      inst.ran[1] = true;
    });
    inst.ctrl.positioner().increment_position(100 * micrometre_t::reference);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ran[1]);
  };
  // Cancel operations by motor error

  // A list of expected errors given a particular motor status
  auto motor_status_and_errors =
      std::array{ std::tuple{ get_bad_status_communication_failure(), err_enum::frequency_drive_communication_fault },
                  std::tuple{ get_bad_status_communication_failure(true), err_enum::frequency_drive_communication_fault },
                  std::tuple{ get_bad_status_missing_phase(), err_enum::frequency_drive_reports_fault },
                  std::tuple{ get_bad_status_missing_phase(true), err_enum::frequency_drive_reports_fault } };
  "run_at_speedratio terminated by a physical motor error"_test = [&](auto& tuple) {
    auto [motor_status, expected_error] = tuple;
    instance<> inst;
    inst.ctrl.update_status(get_good_status_stopped());

    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, [&inst, expected_error](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == expected_error) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    inst.ctrl.update_status(motor_status);
    inst.ctx.run_for(5ms);
    expect(inst.ran[0]);
  } | motor_status_and_errors;

  "run time terminated by a physical motor error"_test = [&](auto& tuple) {
    using tfc::testing::clock;
    auto [motor_status, expected_error] = tuple;
    instance<clock> inst;
    inst.ctrl.update_status(get_good_status_running());
    auto duration = 100 * milli<mp_units::si::second>;
    speedratio_t ratio = 1.0 * percent;
    inst.ctrl.run(ratio, duration, [&inst, expected_error](const std::error_code& err) -> void {
      expect(tfc::motor::motor_enum(err) == expected_error) << err;
      inst.ctx.stop();
      inst.ran[0] = true;
    });
    clock::set_ticks(clock::now() + 80ms);
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(motor_status);
    inst.ctx.run_for(1ms);
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::stop);
    expect(inst.ran[0]);
  } | motor_status_and_errors;

  "convey micrometre terminated by a physical motor error"_test = [&](auto& tuple) {
    instance<> inst;
    auto [motor_status, expected_error] = tuple;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.convey(100 * percent, 1000 * micrometre_t::reference,
                     [&inst, expected_error](const std::error_code& err, const micrometre_t moved) {
                       expect(tfc::motor::motor_enum(err) == expected_error);
                       expect(moved == 700 * micrometre_t::reference);
                       inst.ran[0] = true;
                     });
    inst.ctrl.positioner().increment_position(700 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.update_status(motor_status);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  } | motor_status_and_errors;

  "stop terminated by a physical motor error"_test = [](auto& tuple) {
    instance<> inst;
    auto [motor_status, expected_error] = tuple;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.stop([&inst, expected_error](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == expected_error);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(motor_status);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  } | motor_status_and_errors;

  "quick stop terminated by a physical motor error"_test = [](auto& tuple) {
    instance<> inst;
    auto [motor_status, expected_error] = tuple;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([&inst, expected_error](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == expected_error);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.update_status(motor_status);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  } | motor_status_and_errors;

  "convey micrometre terminated by a physical motor error while stopping"_test = [&](auto& tuple) {
    instance<> inst;
    auto [motor_status, expected_error] = tuple;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.convey(100 * percent, 1000 * micrometre_t::reference,
                     [&inst, expected_error](const std::error_code& err, const micrometre_t moved) {
                       expect(tfc::motor::motor_enum(err) == expected_error);
                       expect(moved == 1000 * micrometre_t::reference);
                       inst.ran[0] = true;
                     });
    inst.ctrl.positioner().increment_position(1000 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.update_status(motor_status);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  } | motor_status_and_errors;
  return EXIT_SUCCESS;
}
