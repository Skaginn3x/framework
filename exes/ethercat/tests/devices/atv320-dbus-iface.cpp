#include "atv320-server-side.hpp"

auto main(int, char const* const* argv) -> int {
  std::array<char const*, 4> args{ argv[0], "--log-level", "trace", "--stdout" };
  tfc::base::init(args.size(), args.data());

  // Good path tests
  "run_at_speedratio terminated by stop"_test = [&] {
    instance inst;
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
    instance inst;
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
    instance inst;
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
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::stop);
    expect(inst.ran[0]);
  };

  "run time happy path"_test = [&] {
    using tfc::testing::clock;
    clock::set_ticks(clock::time_point{});
    instance inst;
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
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::stop);
  };

  "run time"_test = [&] {
    using tfc::testing::clock;
    instance inst;
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
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    expect(inst.ctrl.speed_ratio() == ratio);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::run);
    clock::set_ticks(clock::now() + 9ms);
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.update_status(get_good_status_stopped());
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::stop);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  "move with no reference"_test = [&] {
    instance inst;
    inst.populate_homing_sensor(1 * micrometre_t::reference, false);
    inst.ctrl.move(10 * speedratio_t::reference, 1000 * micrometre_t::reference,
                   [&inst](const std::error_code& err, const micrometre_t moved) {
                     expect(tfc::motor::motor_enum(err) == err_enum::motor_missing_home_reference);
                     expect(moved == 0 * micrometre_t::reference);
                     inst.ran[0] = true;
                     inst.ctx.stop();
                   });
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    expect(inst.ctrl.action() == tfc::ec::cia_402::transition_action::none);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "move with reference"_test = [&] {
    instance inst;
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
    instance inst;
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
  "test reset no fault"_test = [&] {
    using tfc::testing::clock;
    instance inst;
    // set current as reference
    inst.populate_homing_sensor();
    inst.ctrl.update_status(get_good_status_stopped());
    // Since populate homing_sensor sets the homing_displacement amount to 1km we can move just over 1km
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  "test reset fault goes away"_test = [&] {
    using tfc::testing::clock;
    instance inst;
    // set current as reference
    inst.populate_homing_sensor();
    inst.ctrl.update_status(get_bad_status_missing_phase());
    // Since populate homing_sensor sets the homing_displacement amount to 1km we can move just over 1km
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctrl.update_status(get_good_status_stopped());
    clock::set_ticks(clock::now() + 5000ms);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  "test reset fault stays"_test = [&] {
    using tfc::testing::clock;
    instance inst;
    // set current as reference
    inst.populate_homing_sensor();
    inst.ctrl.update_status(get_bad_status_missing_phase());
    // Since populate homing_sensor sets the homing_displacement amount to 1km we can move just over 1km
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    clock::set_ticks(clock::now() + 5000ms);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };

  "reset interupted by reset error gone"_test = [] {
    using tfc::testing::clock;
    instance inst;
    inst.ctrl.update_status(get_bad_status_missing_phase());
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    clock::set_ticks(clock::now() + 5000ms);
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    // This one should also return becuse of resets early return logic
    expect(inst.ran[1]);
  };
  "reset interupted by reset error not gone"_test = [] {
    using tfc::testing::clock;
    instance inst;
    inst.ctrl.update_status(get_bad_status_missing_phase());
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault);
      inst.ran[0] = true;
    });
    clock::set_ticks(clock::now() + 2000ms);
    inst.ctx.run_for(1ms);
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::frequency_drive_reports_fault);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    clock::set_ticks(clock::now() + 5000ms);
    inst.ctx.run_for(1ms);
    expect(inst.ran[1]);
  };
  "reset early return"_test = [] {
    using tfc::testing::clock;
    instance inst;
    inst.ctrl.update_status(get_bad_status_missing_phase());
    inst.ctrl.reset([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::success);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "test stop impl interupted by stop"_test = [&] {
    instance inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(!inst.ran[1]);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[1]);
  };

  "test quick_stop impl interupted by quick_stop"_test = [&] {
    instance inst;
    inst.ctrl.update_status(get_good_status_running());
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(err == std::errc::operation_canceled);
      inst.ran[0] = true;
    });
    inst.ctx.run_for(1ms);
    expect(!inst.ran[0]);
    inst.ctrl.quick_stop([&inst](const std::error_code& err) {
      expect(!err);
      inst.ran[1] = true;
      inst.ctx.stop();
    });
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
    expect(!inst.ran[1]);
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[1]);
  };

  "move_home unconfigured homing sensor"_test = [&] {
    instance inst;
    // Set current as reference
    inst.ctrl.move_home([&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured);
      inst.ran[0] = true;
      inst.ctx.stop();
    });
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    inst.ctrl.positioner().increment_position(0 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "move home already in homing sensor"_test = [&] {
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
  "move unconfigured homing sensor"_test = [&] {
    instance inst;
    // Set current as reference
    inst.ctrl.move(10 * speedratio_t::reference, 10 * micrometre_t::reference,
                   [&inst](const std::error_code& err, const micrometre_t travel) {
                     expect(tfc::motor::motor_enum(err) == err_enum::motor_home_sensor_unconfigured);
                     expect(travel == 0 * micrometre_t::reference);
                     inst.ran[0] = true;
                     inst.ctx.stop();
                   });
    expect(inst.ctrl.speed_ratio() == 0 * speedratio_t::reference);
    inst.ctrl.positioner().increment_position(0 * micrometre_t::reference);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "move cancelled"_test = [] {
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
  "run to positive limit"_test = [] {
    instance inst;
    inst.ctrl.run(100 * percent, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::positioning_positive_limit_reached);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.on_positive_limit_switch(true);
    // Even though a good update we should not clear the error
    inst.ctrl.update_status(get_good_status_running());
    inst.ctx.run_for(1ms);
    // We need to wait till the motor is stopped before the error is propagated
    inst.ctrl.update_status(get_good_status_stopped());
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "run to negative limit"_test = [] {
    instance inst;
    inst.ctrl.run(100 * percent, [&inst](const std::error_code& err) {
      expect(tfc::motor::motor_enum(err) == err_enum::positioning_negative_limit_reached);
      inst.ran[0] = true;
    });
    expect(!inst.ran[0]);
    inst.ctx.run_for(1ms);
    inst.ctrl.on_negative_limit_switch(true);
    inst.ctx.run_for(1ms);
    expect(inst.ran[0]);
  };
  "stoping canceled by convey"_test = [] {
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
    instance inst;
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
