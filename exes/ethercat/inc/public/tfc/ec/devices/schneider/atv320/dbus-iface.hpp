/*
  An interface to allow programs rpc control of the ATV320 drive.
  This is a dbus interface and is more "in depth" then the simple
  ipc-run and speed interface.
*/
#pragma once

#include <string>

#include <mp-units/chrono.h>
#include <mp-units/format.h>
#include <mp-units/quantity.h>
#include <boost/asio.hpp>
#include <boost/asio/experimental/parallel_group.hpp>

#include <tfc/cia/402.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ec/devices/schneider/atv320/pdo.hpp>
#include <tfc/motor/dbus_tags.hpp>
#include <tfc/motor/positioner.hpp>

namespace tfc::ec::devices::schneider::atv320 {
namespace asio = boost::asio;
namespace method = motor::dbus::method;
using speedratio_t = motor::dbus::types::speedratio_t;
using micrometre_t = motor::dbus::types::micrometre_t;
using microsecond_t = motor::dbus::types::microsecond_t;
static constexpr std::string_view impl_name{ "atv320" };

namespace detail {
template <typename completion_token_t>
struct combine_error_code {
  combine_error_code(completion_token_t&& token)
      : self{ std::move(token) }, slot{ asio::get_associated_cancellation_slot(self) } {
    // static_assert(std::is_rvalue_reference_v<completion_token_t>);
  }

  /// The associated cancellation slot type.
  using cancellation_slot_type = asio::cancellation_slot;

  void operator()(auto const& order, std::error_code const& err1, std::error_code const& err2) {
    switch (order[0]) {
      case 0:  // first parallel job has finished
        std::invoke(self, err1);
        break;
      case 1:  // second parallel job has finished
        std::invoke(self, err2);
        break;
      default:
        fmt::println(stderr, "Parallel job has failed, {}", order[0]);
    }
  }

  cancellation_slot_type get_cancellation_slot() const noexcept { return slot; }

  completion_token_t self;
  asio::cancellation_slot slot;
};

template <typename completion_token_t>
combine_error_code(completion_token_t&&) -> combine_error_code<completion_token_t>;

template <typename completion_token_t>
struct drive_error_first {
  drive_error_first(completion_token_t&& token, motor::errors::err_enum& drive_error)
      : drive_err{ drive_error }, self{ std::move(token) }, slot{ asio::get_associated_cancellation_slot(self) } {
    // static_assert(std::is_rvalue_reference_v<completion_token_t>);
  }
  /// The associated cancellation slot type.
  using cancellation_slot_type = asio::cancellation_slot;

  void operator()(auto const& order, std::error_code const& err1, std::error_code const& err2) {
    if (drive_err != motor::errors::err_enum::success) {
      std::invoke(self, motor::motor_error(drive_err));
      return;
    }
    switch (order[0]) {
      case 0:  // first parallel job has finished
        std::invoke(self, err1);
        break;
      case 1:  // second parallel job has finished
        std::invoke(self, err2);
        break;
      default:
        fmt::println(stderr, "Parallel job has failed, {}", order[0]);
    }
  }

  cancellation_slot_type get_cancellation_slot() const noexcept { return slot; }

  motor::errors::err_enum& drive_err;
  completion_token_t self;
  asio::cancellation_slot slot;
};
template <typename completion_token_t>
drive_error_first(completion_token_t&&, motor::errors::err_enum&) -> drive_error_first<completion_token_t>;
}  // namespace detail

// Handy commands
// sudo busctl introspect com.skaginn3x.atv320 /com/skaginn3x/atvmotor
//

template <typename manager_client_t = ipc_ruler::ipc_manager_client,
          template <typename, typename, typename> typename pos_config_t = confman::config,
          typename pos_slot_t = ipc::slot<ipc::details::type_bool, manager_client_t&>>
struct controller {
  controller(std::shared_ptr<sdbusplus::asio::connection> connection, manager_client_t& manager, const uint16_t slave_id)
      : slave_id_{ slave_id }, ctx_{ connection->get_io_context() },
        pos_{ connection, manager, fmt::format("{}_{}", impl_name, slave_id_),
              std::bind_front(&controller::on_homing_sensor, this) } {}

  auto run(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    cancel_pending_operation();
    return run_impl(config_speedratio_,
                    asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto run(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    cancel_pending_operation();
    return run_impl(speedratio, asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto run(speedratio_t speedratio,
           mp_units::QuantityOf<mp_units::isq::time> auto time,
           asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    cancel_pending_operation();
    return run_impl(speedratio, time,
                    asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto run(mp_units::QuantityOf<mp_units::isq::time> auto time,
           asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    cancel_pending_operation();
    return run_impl(config_speedratio_, time,
                    asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto quick_stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    cancel_pending_operation();
    return stop_impl(true, {}, asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    cancel_pending_operation();
    return stop_impl(false, {}, asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto convey(speedratio_t speedratio,
              micrometre_t travel,
              asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    cancel_pending_operation();
    return convey_impl(speedratio, travel,
                       asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto move(speedratio_t speedratio,
            micrometre_t travel,
            asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    cancel_pending_operation();
    return move_impl(speedratio, travel,
                     asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto move_home(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    cancel_pending_operation();
    return move_home_impl(asio::bind_cancellation_slot(cancel_signal_.slot(), std::forward<decltype(token)>(token)));
  }

  auto notify_after(micrometre_t travel, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    cancel_pending_operation();
    return pos_.notify_after(travel, std::forward<decltype(token)>(token));
  }

  auto notify_from_home(micrometre_t position, asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token)
      -> typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    cancel_pending_operation();
    return pos_.notify_from_home(position, std::forward<decltype(token)>(token));
  }

  // Set properties with new status values
  void update_status(const input_t& in) {
    status_word_ = in.status_word;
    motor_frequency_ = in.frequency;
    pos_.freq_update(motor_frequency_);
    if (in.frequency == 0 * mp_units::si::hertz) {
      [[maybe_unused]] boost::system::error_code err{};
      stop_complete_.notify_all(err);
    }
    auto state = status_word_.parse_state();
    using enum motor::errors::err_enum;
    drive_error_ = {};
    if (cia_402::states_e::fault == state || cia_402::states_e::fault_reaction_active == state) {
      drive_error_ = frequency_drive_reports_fault;
    }
    if (drive_error_ != success && in.last_error == lft_e::cnf) {
      drive_error_ = frequency_drive_communication_fault;
    }
    if (drive_error_ != success) {
      drive_error_subscriptable_.notify_all();
    }
  }

  auto set_configured_speedratio(speedratio_t speedratio) {
    auto old_config_speedratio{ config_speedratio_ };
    config_speedratio_ = speedratio;
    // this is not correct, but this operation is non frequent and this simplifies the logic
    if (action_ == cia_402::transition_action::run && speed_ratio_ == old_config_speedratio) {
      speed_ratio_ = config_speedratio_;
    }
  }

  void on_homing_sensor(bool new_v) {
    logger_.trace("New homing sensor value: {}", new_v);
    if (new_v) {
      homing_complete_.notify_all();
    }
  }

  void cancel_pending_operation() { cancel_signal_.emit(asio::cancellation_type::all); }

  auto positioner() noexcept -> auto& { return pos_; }
  auto driver_error() const noexcept -> motor::errors::err_enum { return drive_error_; }
  auto set_motor_nominal_freq(decifrequency nominal_motor_frequency) { motor_nominal_frequency_ = nominal_motor_frequency; }
  speedratio_t speed_ratio() { return speed_ratio_; }

  deciseconds acceleration(const deciseconds configured_acceleration) {
    // TODO influence these parameters depending on action hapening inside dbus-iface.
    return configured_acceleration;
  }

  deciseconds deceleration(const deciseconds configured_deceleration) {
    // TODO influence these parameters depending on action hapening inside dbus-iface.
    return configured_deceleration;
  }

  cia_402::control_word ctrl(bool allow_reset) {
    using enum cia_402::transition_action;
    if (speed_ratio_ < 1 * mp_units::percent && speed_ratio_ > -1 * mp_units::percent && run == action_)
      return transition(status_word_.parse_state(), none, allow_reset);
    return transition(status_word_.parse_state(), action_, allow_reset);
  }

private:
  auto stop_impl(bool use_quick_stop,
                 [[maybe_unused]] std::error_code stop_reason,
                 asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    using enum cia_402::transition_action;
    using enum motor::errors::err_enum;
    action_ = use_quick_stop ? quick_stop : stop;
    speed_ratio_ = 0 * mp_units::percent;
    if (drive_error_ != success) {
      auto const motor_err{ motor::motor_error(drive_error_) };
      logger_.trace("Drive in fault state, command is set as stop but will return the error: {}", motor_err.message());
      return asio::async_compose<std::decay_t<decltype(token)>, void(std::error_code)>(
          [motor_err](auto& self) { self.complete(motor_err); }, token);
    }
    if (motor_frequency_ == 0 * mp_units::si::hertz) {
      logger_.trace("Drive already stopped");
      return asio::async_compose<std::decay_t<decltype(token)>, void(std::error_code)>([](auto& self) { self.complete({}); },
                                                                                       token);
    }
    return asio::async_compose<std::decay_t<decltype(token)>, void(std::error_code)>(
        [this, stop_reason, first_call = true](auto& self, std::error_code err = {}) mutable {
          if (first_call) {
            first_call = false;
            asio::experimental::make_parallel_group(
                [&](auto inner_token) { return drive_error_subscriptable_.async_wait(inner_token); },
                [&](auto inner_token) { return stop_complete_.async_wait(inner_token); })
                .async_wait(asio::experimental::wait_for_one(), detail::drive_error_first(std::move(self), drive_error_));
            return;
          }
          if (stop_reason) {
            self.complete(stop_reason);
            return;
          }
          self.complete(err);
        },
        token);
  }

  auto run_impl(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    using enum motor::errors::err_enum;
    if (drive_error_ != success) {
      logger_.trace("Drive in fault state, cannot run");
      return asio::async_compose<std::decay_t<decltype(token)>, void(std::error_code)>(
          [this](auto& self) { self.complete(motor::motor_error(drive_error_)); }, token);
    }
    if (speedratio < -100 * mp_units::percent || speedratio > 100 * mp_units::percent) {
      logger_.trace("Speedratio not within range [-100,100], value: {}", speedratio);
      return asio::async_compose<std::decay_t<decltype(token)>, void(std::error_code)>(
          [](auto& self) { self.complete(motor::motor_error(speedratio_out_of_range)); }, token);
    }
    logger_.trace("Run motor at speedratio: {}", speedratio);
    action_ = cia_402::transition_action::run;
    speed_ratio_ = speedratio;
    return asio::async_compose<std::decay_t<decltype(token)>, void(std::error_code)>(
        [this, first_call = true](auto& self, std::error_code err = {}) mutable {
          if (first_call) {
            first_call = false;
            asio::experimental::make_parallel_group(
                [&](auto inner_token) { return drive_error_subscriptable_.async_wait(inner_token); },
                [&](auto inner_token) { return run_blocker_.async_wait(inner_token); })
                .async_wait(asio::experimental::wait_for_one(), detail::drive_error_first(std::move(self), drive_error_));
            return;
          }
          self.complete(err);
        },
        token);
  }

  // todo bit duplicated from convey_impl
  auto run_impl(speedratio_t speedratio,
                mp_units::QuantityOf<mp_units::isq::time> auto time,
                asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    using signature_t = void(std::error_code, micrometre_t);
    enum struct state_e : std::uint8_t { run_until_notify = 0, wait_till_stop, complete };
    auto const is_positive{ speedratio > 0L * speedratio_t::reference };
    auto const pos{ pos_.position() };
    auto timer{ std::make_shared<asio::steady_timer>(ctx_) };
    return asio::async_compose<decltype(token), signature_t>(
        [this, speedratio, time, timer, state = state_e::run_until_notify, is_positive, pos](
            auto& self, std::error_code err = {}) mutable {
          using enum motor::errors::err_enum;
          switch (state) {
            case state_e::run_until_notify: {
              state = state_e::wait_till_stop;
              logger_.trace("Run time: {}, current position: {}", time, pos);
              if (time == 0L * decltype(time)::reference) {
                self.complete({}, 0L * micrometre_t::reference);
                return;
              }

              asio::experimental::make_parallel_group(
                  [&](auto inner_token) { return run_impl(is_positive ? speedratio : -speedratio, inner_token); },
                  [&](auto inner_token) {
                    timer->expires_after(mp_units::to_chrono_duration(time));
                    return timer->async_wait(inner_token);
                  })
                  .async_wait(asio::experimental::wait_for_one(), detail::combine_error_code(std::move(self)));
              return;
            }
            case state_e::wait_till_stop: {
              state = state_e::complete;
              // This is only called if another invocation has not taken control of the motor
              // stopping the motor now would be counter productive as somebody is using it.
              if (err == std::errc::operation_canceled) {
                self(err);  // calling complete
                return;
              }
              // Todo this stops quickly :-)
              stop_impl(true, err, std::move(self));
              return;
            }
            case state_e::complete: {
              auto const actual_travel{ is_positive ? (pos_.position() - pos).force_in(micrometre_t::reference)
                                                    : -(pos - pos_.position()).force_in(micrometre_t::reference) };

              if (err) {
                logger_.warn("Convey failed: {}", err.message());
              }
              logger_.trace("Actual travel: {}, after time: {}", actual_travel, time);
              self.complete(err, actual_travel);
            }
          }
        },
        token);
  }

  auto convey_impl(speedratio_t speedratio,
                   micrometre_t travel,
                   asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    using signature_t = void(std::error_code, micrometre_t);
    enum struct state_e : std::uint8_t { run_until_notify = 0, wait_till_stop, complete };
    auto const is_positive{ travel > 0L * micrometre_t::reference };
    auto const pos{ pos_.position() };
    return asio::async_compose<decltype(token), signature_t>(
        [this, speedratio, travel, state = state_e::run_until_notify, is_positive, pos](auto& self,
                                                                                        std::error_code err = {}) mutable {
          using enum motor::errors::err_enum;
          switch (state) {
            case state_e::run_until_notify: {
              state = state_e::wait_till_stop;
              logger_.trace("Target displacement: {}, current position: {}", travel, pos);
              if (travel == 0L * micrometre_t::reference) {
                self.complete({}, 0L * micrometre_t::reference);
                return;
              }

              asio::experimental::make_parallel_group(
                  [&](auto inner_token) { return run_impl(is_positive ? speedratio : -speedratio, inner_token); },
                  [&](auto inner_token) { return pos_.notify_after(travel, inner_token); })
                  .async_wait(asio::experimental::wait_for_one(), detail::combine_error_code(std::move(self)));
              return;
            }
            case state_e::wait_till_stop: {
              state = state_e::complete;
              // This is only called if another invocation has not taken control of the motor
              // stopping the motor now would be counter productive as somebody is using it.
              if (err == std::errc::operation_canceled) {
                self(err);  // calling complete
                return;
              }
              // Todo this stops quickly :-)
              stop_impl(true, err, std::move(self));
              return;
            }
            case state_e::complete: {
              auto const actual_travel{ is_positive ? (pos_.position() - pos).force_in(micrometre_t::reference)
                                                    : -(pos - pos_.position()).force_in(micrometre_t::reference) };

              if (err) {
                logger_.warn("Convey failed: {}", err.message());
              }
              logger_.trace("Actual travel: {}, where target was: {}", actual_travel, travel);
              self.complete(err, actual_travel);
            }
          }
        },
        token);
  }

  auto move_impl(speedratio_t speedratio,
                 micrometre_t placement,
                 asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    using signature_t = void(std::error_code, micrometre_t);
    enum struct state_e : std::uint8_t { move_until_notify = 0, wait_till_stop, complete };
    micrometre_t pos_from_home{ pos_.position_from_home().force_in(micrometre_t::reference) };
    bool const is_positive{ pos_from_home < placement };
    auto const resolution{ pos_.resolution() };
    return asio::async_compose<decltype(token), signature_t>(
        [this, speedratio, placement, pos_from_home, is_positive, resolution, state = state_e::move_until_notify](
            auto& self, std::error_code err = {}) mutable {
          using enum motor::errors::err_enum;
          switch (state) {
            case state_e::move_until_notify: {
              state = state_e::wait_till_stop;
              // Get our distance from the homing reference
              if (!pos_.homing_enabled() || pos_.error() == motor_missing_home_reference) {
                logger_.trace("{}", motor_missing_home_reference);
                return self.complete(motor::motor_error(motor_missing_home_reference), pos_from_home);
              }
              if (pos_.would_need_homing(pos_from_home - placement)) {
                logger_.trace("Woud need homing if motor were to move to: {}", placement);
                return self.complete(motor::motor_error(motor_missing_home_reference), pos_from_home);
              }
              logger_.trace("Target placement: {}, currently at: {}, with resolution: {}", placement, pos_from_home,
                            resolution);
              if (placement + resolution >= pos_from_home && placement < pos_from_home + resolution) {
                logger_.trace("Currently within resolution of current position");
                return self.complete({}, placement);
              }
              asio::experimental::make_parallel_group(
                  [&](auto inner_token) { return run_impl(is_positive ? speedratio : -speedratio, inner_token); },
                  [&](auto inner_token) { return pos_.notify_from_home(placement, inner_token); })
                  .async_wait(asio::experimental::wait_for_one(), detail::combine_error_code(std::move(self)));
              return;
            }
            case state_e::wait_till_stop: {
              state = state_e::complete;
              // This is only called if another invocation has not taken control of the motor
              // stopping the motor now would be counter productive as somebody is using it.
              if (err == std::errc::operation_canceled) {
                std::invoke(self, err);  // calling complete of this lambda
                return;
              }
              // Todo this stops quickly :-)
              // imagining 6 DOF robot arm, moving towards a specific radian in 3D space, it would depend on where the arm
              // is going so a single config variable for deceleration would not be sufficient, I propose to add
              // it(deceleration time) to the API call when needed implementing deceleration would propably be best to be
              // controlled by this code not the atv itself, meaning decrement given speedratio to 1% (using the given
              // deceleration time) when 1% is reached next
              pos_from_home = pos_.position_from_home().force_in(micrometre_t::reference);
              logger_.trace("Will stop motor, now at: {}, where target was: {}", pos_from_home, placement);
              [[maybe_unused]] auto slot = asio::get_associated_cancellation_slot(self);
              stop_impl(true, err, std::move(self));
              return;
            }
            case state_e::complete: {
              if (err) {
                logger_.warn("Move failed: {}", err.message());
              }
              pos_from_home = pos_.position_from_home().force_in(micrometre_t::reference);
              logger_.trace("Motor should be stopped, now at: {}, where target was: {}", pos_from_home, placement);
              self.complete(err, pos_from_home);
            }
          }
        },
        token);
  }

  auto move_home_impl(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    using signature_t = void(std::error_code);
    enum struct state_e : std::uint8_t { move_until_sensor = 0, wait_till_stop, complete };
    auto const& travel_speed{ pos_.homing_travel_speed() };
    auto const& homing_sensor{ pos_.homing_sensor() };
    return asio::async_compose<decltype(token), signature_t>(
        [this, &travel_speed, &homing_sensor, state = state_e::move_until_sensor](auto& self,
                                                                                  std::error_code err = {}) mutable {
          using enum motor::errors::err_enum;
          switch (state) {
            case state_e::move_until_sensor: {
              state = state_e::wait_till_stop;
              if (!travel_speed.has_value() || !homing_sensor.has_value()) {
                self.complete(motor::motor_error(motor_home_sensor_unconfigured));
                return;
              }
              logger_.trace("Homing motor at speed: {}, currently positioned at: {}", travel_speed.value(),
                            pos_.position_from_home());
              if (homing_sensor->value().has_value() && homing_sensor->value().value()) {
                // todo move out of sensor and back to sensor
                auto const pos{ pos_.position() };
                logger_.info("Already at home, storing position: {}", pos);
                pos_.home(pos);
                self.complete({});
                return;
              }
              asio::experimental::make_parallel_group(
                  [&](auto inner_token) { return run_impl(travel_speed.value(), inner_token); },
                  [&](auto inner_token) { return homing_complete_.async_wait(inner_token); })
                  .async_wait(asio::experimental::wait_for_one(), detail::combine_error_code(std::move(self)));
              return;
            }
            case state_e::wait_till_stop: {
              state = state_e::complete;
              // let's verify that we are not positioned at home
              // if we are not in sensor, check whether we are cancelled
              if (!homing_sensor->value().value() && err == std::errc::operation_canceled) {
                logger_.trace("Move home got cancelled");
                self.complete(err);
                return;
              }
              stop_impl(true, err, std::move(self));
              auto const pos{ pos_.position() };
              logger_.trace("Storing home position: {}", pos);
              pos_.home(pos);
              return;
            }
            case state_e::complete: {
              if (err) {
                logger_.warn("Homing failed: {}", err.message());
              }
              self.complete(err);
            }
          }
        },
        token);
  }

  std::uint16_t slave_id_;
  asio::io_context& ctx_;
  motor::positioner::positioner<mp_units::si::metre, manager_client_t&, pos_config_t, pos_slot_t> pos_;
  tfc::asio::condition_variable<asio::any_io_executor> run_blocker_{ ctx_.get_executor() };
  tfc::asio::condition_variable<asio::any_io_executor> stop_complete_{ ctx_.get_executor() };
  tfc::asio::condition_variable<asio::any_io_executor> drive_error_subscriptable_{ ctx_.get_executor() };
  tfc::asio::condition_variable<asio::any_io_executor> homing_complete_{ ctx_.get_executor() };
  asio::cancellation_signal cancel_signal_{};
  logger::logger logger_{ fmt::format("{}_{}", impl_name, slave_id_) };

  // Motor control parameters
  cia_402::transition_action action_{ cia_402::transition_action::none };
  speedratio_t speed_ratio_{ 0.0 * mp_units::percent };
  cia_402::status_word status_word_{};
  decifrequency motor_nominal_frequency_{};  // Indication if this is a 50Hz motor or 120Hz motor. That number has an effect

  // Motor config
  speedratio_t config_speedratio_{ 0.0 * mp_units::percent };

  // Motor status
  motor::errors::err_enum drive_error_{};
  decifrequency_signed motor_frequency_{};
};

struct dbus_iface {
  // Properties
  const std::string connected_peer{ "connected_peer" };
  const std::string frequency{ "frequency" };
  const std::string state_402{ "state_402" };
  const std::string current{ "current" };
  const std::string last_error{ "last_error" };
  const std::string hmis{ "hmis" };  // todo change to more readable form

  dbus_iface(const dbus_iface&) = delete;
  dbus_iface(dbus_iface&&) = delete;
  auto operator=(const dbus_iface&) -> dbus_iface& = delete;
  auto operator=(dbus_iface&&) -> dbus_iface& = delete;
  ~dbus_iface() = default;

  dbus_iface(controller<>& ctrl, std::shared_ptr<sdbusplus::asio::connection> connection, const uint16_t slave_id)
      : ctx_(connection->get_io_context()), slave_id_{ slave_id }, ctrl_{ ctrl }, manager_(connection),

        logger_(fmt::format("{}_{}", impl_name, slave_id_)) {
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection, false);
    dbus_interface_ = object_server_->add_unique_interface(std::string{ motor::dbus::path },
                                                           motor::dbus::make_interface_name(impl_name, slave_id_));

    /**
     * Never set long_living_ping from a program. This parameter is only here
     * for convinience while testing motor directions and fault finding during
     * commisioning.
     **/
    dbus_interface_->register_method(
        std::string{ method::ping }, [this](const sdbusplus::message_t& msg, bool long_living_ping) -> bool {
          const std::string incoming_peer = msg.get_sender();
          const bool new_peer = incoming_peer != peer_;
          // This is the same peer or we have no peer
          if (incoming_peer == peer_ || (new_peer && peer_ == "")) {
            if (new_peer) {
              peer_ = incoming_peer;
              dbus_interface_->set_property(connected_peer, peer_);
            }
            timeout_.cancel();
            timeout_.expires_after(long_living_ping ? std::chrono::hours(1) : std::chrono::milliseconds(750));
            timeout_.async_wait([this](std::error_code err) {
              if (err)
                return;  // The timer was canceled or deconstructed.
              // Stop the drive from running since the peer has disconnected
              ctrl_.stop([this](const std::error_code& time_err) {
                // TODO: IS THIS RIGHT
                if (time_err) {
                  logger_.error("Stop failed after peer disconnect : {}", time_err.message());
                }
              });
              peer_ = "";
              dbus_interface_->set_property(connected_peer, peer_);
            });
            return true;
          } else {
            // Peer rejected
            return false;
          }
        });

    dbus_interface_->register_method(
        std::string{ method::run },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg) -> motor::errors::err_enum {
          using enum motor::errors::err_enum;
          if (!validate_peer(msg.get_sender())) {
            return permission_denied;
          }
          return motor::motor_enum(ctrl_.run(yield));
        });

    dbus_interface_->register_method(std::string{ method::run_at_speedratio },
                                     [this](asio::yield_context yield, const sdbusplus::message_t& msg,
                                            speedratio_t speedratio) -> motor::errors::err_enum {
                                       using enum motor::errors::err_enum;
                                       if (!validate_peer(msg.get_sender())) {
                                         return permission_denied;
                                       }
                                       return motor::motor_enum(ctrl_.run(speedratio, yield));
                                     });

    dbus_interface_->register_method(std::string{ method::run_at_speedratio_microsecond },
                                     [this](asio::yield_context yield, const sdbusplus::message_t& msg,
                                            speedratio_t speedratio, microsecond_t microsecond) -> motor::errors::err_enum {
                                       using enum motor::errors::err_enum;
                                       if (!validate_peer(msg.get_sender())) {
                                         return permission_denied;
                                       }
                                       auto [err, distance_travelled_unused]{ ctrl_.run(speedratio, microsecond, yield) };
                                       return motor::motor_enum(err);
                                     });

    dbus_interface_->register_method(std::string{ method::run_microsecond },
                                     [this](asio::yield_context yield, const sdbusplus::message_t& msg,
                                            microsecond_t microsecond) -> motor::errors::err_enum {
                                       using enum motor::errors::err_enum;
                                       if (!validate_peer(msg.get_sender())) {
                                         return permission_denied;
                                       }
                                       auto [err, distance_travelled_unused]{ ctrl_.run(microsecond, yield) };
                                       return motor::motor_enum(err);
                                     });

    dbus_interface_->register_method(
        std::string{ method::notify_after_micrometre },
        [this](asio::yield_context yield, micrometre_t distance) -> std::tuple<motor::errors::err_enum> {
          auto err{ ctrl_.notify_after(distance, yield) };
          return motor::motor_enum(err);
        });

    dbus_interface_->register_method(
        std::string{ method::stop },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg) -> motor::errors::err_enum {
          using enum motor::errors::err_enum;
          if (!validate_peer(msg.get_sender())) {
            return permission_denied;
          }
          return motor::motor_enum(ctrl_.stop(yield));
        });

    dbus_interface_->register_method(
        std::string{ method::quick_stop },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg) -> motor::errors::err_enum {
          // todo duplicate
          using enum motor::errors::err_enum;
          if (!validate_peer(msg.get_sender())) {
            return permission_denied;
          }
          return motor::motor_enum(ctrl_.quick_stop(yield));
        });

    dbus_interface_->register_method(
        std::string{ method::move_home },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg) -> motor::errors::err_enum {
          if (!validate_peer(msg.get_sender())) {
            return motor::errors::err_enum::permission_denied;
          }
          return motor::motor_enum(ctrl_.move_home(yield));
        });

    // returns { error_code, actual displacement }
    dbus_interface_->register_method(std::string{ method::convey_micrometre },
                                     [this](asio::yield_context yield, sdbusplus::message_t const& msg,
                                            micrometre_t travel) -> std::tuple<motor::errors::err_enum, micrometre_t> {
                                       using enum motor::errors::err_enum;
                                       if (!validate_peer(msg.get_sender())) {
                                         return std::make_tuple(permission_denied, 0L * micrometre_t::reference);
                                       }
                                       auto [err, actual_displacement]{ ctrl_.convey(config_speedratio_, travel, yield) };
                                       return std::make_tuple(motor::motor_enum(err), actual_displacement);
                                     });

    // returns { error_code, absolute position relative to home }
    dbus_interface_->register_method(std::string{ method::move_speedratio_micrometre },
                                     [this](asio::yield_context yield, speedratio_t speedratio,
                                            micrometre_t travel) -> std::tuple<motor::errors::err_enum, micrometre_t> {
                                       auto [err, traveled]{ ctrl_.move(speedratio, travel, yield) };
                                       return std::make_tuple(motor::motor_enum(err), traveled);
                                     });

    // returns { error_code, absolute position relative to home }
    dbus_interface_->register_method(
        std::string{ method::move_micrometre },
        [this](asio::yield_context yield, micrometre_t placement) -> std::tuple<motor::errors::err_enum, micrometre_t> {
          auto [err, final_placement]{ ctrl_.move(config_speedratio_, placement, yield) };
          return std::make_tuple(motor::motor_enum(err), final_placement);
        });

    dbus_interface_->register_property<std::string>(connected_peer, peer_);
    dbus_interface_->register_property<std::string>(state_402, "");
    dbus_interface_->register_property<std::string>(hmis, "");
    dbus_interface_->register_property<std::string>(last_error, "");
    dbus_interface_->register_property<double>(current, 0.0);

    dbus_interface_->initialize();
  }

  auto set_configured_speedratio(speedratio_t speedratio) { config_speedratio_ = speedratio; }

  // auto set_motor_nominal_freq(decifrequency nominal_motor_frequency) { motor_nominal_frequency_ = nominal_motor_frequency;
  // }

  auto validate_peer(std::string_view incoming_peer) -> bool {
    if (incoming_peer != peer_) {
      logger_.warn("Peer rejected: {}", incoming_peer);
      return false;
    }
    return true;
  }

  // Set properties with new status values
  void update_status(const input_t& in) {
    if (in.status_word.parse_state() != last_in_.status_word.parse_state()) {
      dbus_interface_->set_property(state_402, in.status_word.parse_state());
    }
    if (in.drive_state != last_in_.drive_state) {
      dbus_interface_->set_property(hmis, format_as(in.drive_state));
    }
    if (in.last_error != last_in_.last_error) {
      dbus_interface_->set_property(last_error, format_as(in.last_error));
    }
    if (in.current != last_in_.current) {
      dbus_interface_->set_property(current, static_cast<double>(in.current) / 10.0);
    }
    last_in_ = in;
  }

  asio::io_context& ctx_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;  // todo is this needed, if so why, I am curious
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  asio::steady_timer timeout_{ ctx_ };
  std::string peer_{ "" };

  const uint16_t slave_id_;
  controller<>& ctrl_;
  speedratio_t config_speedratio_{ 0.0 * mp_units::percent };
  motor::errors::err_enum drive_error_{};

  ipc_ruler::ipc_manager_client manager_;
  // motor::positioner::positioner<> pos_;
  logger::logger logger_;
  input_t last_in_{};

  /**
   * \brief has_peer
   * \return true if the dbus interface is being used to control the drive.
   */
  bool has_peer() { return peer_ != ""; }
};
}  // namespace tfc::ec::devices::schneider::atv320
