/*
  An interface to allow programs rpc control of the ATV320 drive.
  This is a dbus interface and is more "in depth" then the simple
  ipc-run and speed interface.
*/
#pragma once

#include <string>

#include <mp-units/format.h>
#include <mp-units/quantity.h>
#include <boost/asio.hpp>
#include <boost/asio/experimental/parallel_group.hpp>

#include <tfc/cia/402.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ec/devices/schneider/atv320/pdo.hpp>
#include <tfc/ec/devices/schneider/atv320/speedratio.hpp>
#include <tfc/motor/dbus_tags.hpp>
#include <tfc/motor/positioner.hpp>

namespace tfc::ec::devices::schneider::atv320 {
namespace asio = boost::asio;
namespace method = motor::dbus::method;
using speedratio_t = motor::dbus::types::speedratio_t;
using micrometre_t = motor::dbus::types::micrometre_t;
static constexpr std::string_view impl_name{ "atv320" };

namespace detail {
template <typename completion_token_t>
struct combine_error_code {
  combine_error_code(completion_token_t&& token) : self{ std::move(token) }, slot{ asio::get_associated_cancellation_slot(self) } {
    // static_assert(std::is_rvalue_reference_v<completion_token_t>);
  }
  /// The associated cancellation slot type.
  using cancellation_slot_type = asio::cancellation_slot;

  void operator()(auto const& order, std::error_code const& err1,
                                                       std::error_code const& err2) {
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

  cancellation_slot_type get_cancellation_slot() const noexcept
  {
    return slot;
  }

  completion_token_t self;
  asio::cancellation_slot slot;
};
template <typename completion_token_t>
combine_error_code(completion_token_t&&) -> combine_error_code<completion_token_t>;
}  // namespace detail

// Handy commands
// sudo busctl introspect com.skaginn3x.atv320 /com/skaginn3x/atvmotor
//

template <typename manager_client_t, template <typename, typename, typename> typename pos_config_t = confman::config,
          typename pos_slot_t = ipc::slot<ipc::details::type_bool, manager_client_t>>
struct controller {
  controller(std::shared_ptr<sdbusplus::asio::connection> connection, manager_client_t manager, const uint16_t slave_id)
      : slave_id_{ slave_id }, ctx_{ connection->get_io_context() },
        pos_{ connection, manager, fmt::format("{}_{}", impl_name, slave_id_),
              std::bind_front(&controller::on_homing_sensor, this) } {}

  auto run_at_speedratio(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    cancel_pending_operation();
    return run_at_speedratio_impl(speedratio,
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

  auto convey(speedratio_t speedratio, micrometre_t travel,
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
      drive_error_ = motor_general_error;
    }
    if (drive_error_ != success && in.last_error == lft_e::cnf) {
      drive_error_ = motor_communication_fault;
    }
    if (drive_error_ != success) {
      // big TODO propagate drive error to pending operation
      cancel_pending_operation();
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

private:
  auto stop_impl(bool use_quick_stop, std::error_code stop_reason, asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    using enum cia_402::transition_action;
    action_ = use_quick_stop ? quick_stop : stop;
    speed_ratio_ = 0 * mp_units::percent;
    if (motor_frequency_ == 0 * mp_units::si::hertz) {
      logger_.trace("Drive in fault state, cannot run");
      return asio::async_compose<std::decay_t<decltype(token)>, void(std::error_code)>(
          [](auto& self) { self.complete({}); }, token);
    }
    auto cancelation_slot{ asio::get_associated_cancellation_slot(token) };

    return stop_complete_.async_wait(asio::bind_cancellation_slot(cancelation_slot, [tok = std::forward<decltype(token)>(token), stop_reason](std::error_code err) mutable {
      if (stop_reason) {
        std::invoke(tok, stop_reason);
      }
      std::invoke(tok, err);
    }));
  }

  auto run_at_speedratio_impl(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
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
    return run_blocker_.async_wait(std::forward<decltype(token)>(token));
  }

  auto convey_impl(speedratio_t speedratio, micrometre_t travel,
                              asio::completion_token_for<void(std::error_code, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code, micrometre_t)>::return_type {
    using signature_t = void(std::error_code, micrometre_t);
    enum struct state_e : std::uint8_t { run_until_notify = 0, wait_till_stop, complete };
    auto const is_positive{ travel > 0L * micrometre_t::reference };
    auto const pos{ pos_.position() };
    return asio::async_compose<decltype(token), signature_t>(
        [this, speedratio, travel, state = state_e::run_until_notify, is_positive, pos](auto& self, std::error_code err = {}) mutable {
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
                  [&](auto inner_token) {
                    return run_at_speedratio_impl(is_positive ? speedratio : -speedratio, inner_token);
                  },
                  [&](auto inner_token) { return pos_.notify_after(travel, inner_token); })
                  .async_wait(asio::experimental::wait_for_one(),
                              detail::combine_error_code(std::move(self)));
              return;
            }
            case state_e::wait_till_stop: {
              state = state_e::complete;
              // This is only called if another invocation has not taken control of the motor
              // stopping the motor now would be counter productive as somebody is using it.
              if (err == std::errc::operation_canceled) {
                self(err); // calling complete
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
        [this, speedratio, placement, pos_from_home, is_positive, resolution, state = state_e::move_until_notify](auto& self,
                                                                                         std::error_code err = {}) mutable {
          using enum motor::errors::err_enum;
          switch (state) {
            case state_e::move_until_notify: {
              state = state_e::wait_till_stop;
              // Get our distance from the homing reference
              if (!pos_.homing_enabled() || pos_.error() == motor_missing_home_reference) {
                logger_.trace("{}", motor_missing_home_reference);
                return self.complete(motor::motor_error(motor_missing_home_reference), pos_from_home);
              }
              logger_.trace("Target placement: {}, currently at: {}, with resolution: {}", placement, pos_from_home,
                            resolution);
              if (placement + resolution >= pos_from_home && placement < pos_from_home + resolution) {
                logger_.trace("Currently within resolution of current position");
                return self.complete({}, placement);
              }
              asio::experimental::make_parallel_group(
                  [&](auto inner_token) { return run_at_speedratio_impl(is_positive ? speedratio : -speedratio, inner_token); },
                  [&](auto inner_token) { return pos_.notify_from_home(placement, inner_token); })
                  .async_wait(asio::experimental::wait_for_one(), detail::combine_error_code(std::move(self)));
              return;
            }
            case state_e::wait_till_stop: {
              state = state_e::complete;
              // This is only called if another invocation has not taken control of the motor
              // stopping the motor now would be counter productive as somebody is using it.
              if (err == std::errc::operation_canceled) {
                std::invoke(self, err); // calling complete of this lambda
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
    return asio::async_compose<decltype(token), signature_t>([this, travel_speed, homing_sensor, state = state_e::move_until_sensor](auto& self, std::error_code err = {}) mutable {
      using enum motor::errors::err_enum;
      switch (state) {
        case state_e::move_until_sensor: {
          state = state_e::wait_till_stop;
          if (!travel_speed.has_value() || !homing_sensor.has_value()) {
            self.complete(motor::motor_error(motor_home_sensor_unconfigured));
            return;
          }
          logger_.trace("Homing motor at speed: {}, currently positioned at: {}", travel_speed.value(), pos_.position_from_home());
          if (homing_sensor->value().has_value() && homing_sensor->value().value()) {
            // todo move out of sensor and back to sensor
            auto const pos{ pos_.position() };
            logger_.info("Already at home, storing position: {}", pos);
            pos_.home(pos);
            self.complete({});
            return;
          }
          asio::experimental::make_parallel_group(
            [&](auto inner_token) { return run_at_speedratio_impl(travel_speed.value(), inner_token); },
            [&](auto inner_token) { return homing_complete_.async_wait(inner_token); })
            .async_wait(asio::experimental::wait_for_one(), detail::combine_error_code(std::move(self)));
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
        }
        case state_e::complete: {
          if (err) {
            logger_.warn("Homing failed: {}", err.message());
          }
          self.complete(err);
        }
      }
    }, token);
  }

  std::uint16_t slave_id_;
  asio::io_context& ctx_;
  motor::positioner::positioner<mp_units::si::metre, manager_client_t, pos_config_t, pos_slot_t> pos_;
  tfc::asio::condition_variable<asio::any_io_executor> run_blocker_{ ctx_.get_executor() };
  tfc::asio::condition_variable<asio::any_io_executor> stop_complete_{ ctx_.get_executor() };
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
  static constexpr std::string_view connected_peer{ "connected_peer" };
  static constexpr std::string_view frequency{ "frequency" };
  static constexpr std::string_view state_402{ "state_402" };
  static constexpr std::string_view hmis{ "hmis" };  // todo change to more readable form

  dbus_iface(const dbus_iface&) = delete;
  dbus_iface(dbus_iface&&) = delete;
  auto operator=(const dbus_iface&) -> dbus_iface& = delete;
  auto operator=(dbus_iface&&) -> dbus_iface& = delete;
  ~dbus_iface() = default;

  auto convey_micrometre(micrometre_t travel,
                         asio::completion_token_for<void(motor::errors::err_enum, micrometre_t)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(motor::errors::err_enum, micrometre_t)>::return_type {
    using signature_t = void(motor::errors::err_enum, micrometre_t);
    enum struct state_e : std::uint8_t { run_until_notify = 0, wait_till_stop, complete };
    auto const is_positive{ travel > 0L * micrometre_t::reference };
    auto const pos{ pos_.position() };
    return asio::async_compose<decltype(token), signature_t>(
        [this, travel, state = state_e::run_until_notify, is_positive, pos](auto& self, std::error_code err = {}) mutable {
          using enum motor::errors::err_enum;
          switch (state) {
            case state_e::run_until_notify:
              state = state_e::wait_till_stop;
              logger_.trace("Target displacement: {}, current position: {}", travel, pos);
              if (travel == 0L * micrometre_t::reference) {
                self.complete(success, 0L * micrometre_t::reference);
                return;
              }
              cancel_pending_operation();

              asio::experimental::make_parallel_group(
                  [&](auto inner_token) {
                    return run_at_speedratio(is_positive ? config_speedratio_ : -config_speedratio_, inner_token);
                  },
                  [&](auto inner_token) { return pos_.notify_after(travel, inner_token); })
                  .async_wait(asio::experimental::wait_for_one(), detail::combine_error_code(std::move(self)));
              return;
            case state_e::wait_till_stop:
              state = state_e::complete;
              // This is only called if another invocation has not taken control of the motor
              // stopping the motor now would be counter productive as somebody is using it.
              if (err != std::errc::operation_canceled) {
                // Todo this stops quickly :-)
                quick_stop(std::move(self));
                return;
              }
              self(err);
              return;
            case state_e::complete:
              auto const actual_travel{ is_positive ? (pos_.position() - pos).force_in(micrometre_t::reference)
                                                    : -(pos - pos_.position()).force_in(micrometre_t::reference) };

              if (err) {
                logger_.warn("Convey failed: {}", err.message());
                self.complete(motor::motor_enum(err), actual_travel);
                return;
              }
              logger_.trace("Actual travel: {}, where target was: {}", actual_travel, travel);
              self.complete(success, actual_travel);
          }
        },
        token);
  }

  dbus_iface(std::shared_ptr<sdbusplus::asio::connection> connection, const uint16_t slave_id)
      : ctx_(connection->get_io_context()), slave_id_{ slave_id },
        manager_(connection),
        pos_{ connection, manager_, fmt::format("{}_{}", impl_name, slave_id_), std::bind_front(&dbus_iface::on_homing_sensor, this) },
        logger_(fmt::format("{}_{}", impl_name, slave_id_)) {
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection, false);
    dbus_interface_ = object_server_->add_unique_interface(std::string{ motor::dbus::path },
                                                           motor::dbus::make_interface_name(impl_name, slave_id_));
    dbus_interface_->register_method(std::string{ method::ping }, [this](const sdbusplus::message_t& msg) -> bool {
      std::string incoming_peer = msg.get_sender();
      bool new_peer = incoming_peer != peer_;
      // This is the same peer or we have no peer
      if (incoming_peer == peer_ || (new_peer && peer_ == "")) {
        if (new_peer) {
          peer_ = incoming_peer;
          dbus_interface_->signal_property(std::string{ connected_peer });
        }
        timeout_.cancel();
        timeout_.expires_after(std::chrono::days(750));  // todo revert
        timeout_.async_wait([this](std::error_code err) {
          if (err)
            return;  // The timer was canceled or deconstructed.
          // Stop the drive from running since the peer has disconnected
          action = cia_402::transition_action::none;
          speed_ratio_ = 0.0 * mp_units::percent;
          peer_ = "";
          dbus_interface_->signal_property(std::string{ connected_peer });
        });
        return true;
      } else {
        // Peer rejected
        return false;
      }
    });

    dbus_interface_->register_method(std::string{ method::run_at_speedratio },
                                     [this](asio::yield_context yield, const sdbusplus::message_t& msg,
                                            speedratio_t speedratio) -> motor::errors::err_enum {
                                       using enum motor::errors::err_enum;
                                       if (!validate_peer(msg.get_sender())) {
                                         return permission_denied;
                                       }
                                       cancel_pending_operation();
                                       std::error_code err{ run_at_speedratio(speedratio, std::move(yield)) };
                                       return motor::motor_enum(err);
                                     });
    dbus_interface_->register_method(
        std::string{ method::notify_after_micrometre },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg, micrometre_t distance) {
          std::string incoming_peer = msg.get_sender();
          pos_.notify_after(distance, yield);
        });

    dbus_interface_->register_method(
        std::string{ method::stop },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg) -> motor::errors::err_enum {
          using enum motor::errors::err_enum;
          if (!validate_peer(msg.get_sender())) {
            return permission_denied;
          }
          cancel_pending_operation();
          std::error_code err{ stop(yield) };
          return motor::motor_enum(err);
        });

    dbus_interface_->register_method(
        std::string{ method::quick_stop },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg) -> motor::errors::err_enum {
          // todo duplicate
          using enum motor::errors::err_enum;
          if (!validate_peer(msg.get_sender())) {
            return permission_denied;
          }
          cancel_pending_operation();
          std::error_code err{ quick_stop(yield) };
          return motor::motor_enum(err);
        });

    dbus_interface_->register_method(std::string{ method::move_home }, [this](asio::yield_context yield,
                                                                              const sdbusplus::message_t& msg) {
      if (!validate_peer(msg.get_sender())) {
        return;
      }
      cancel_pending_operation();
      auto const& travel_speed{ pos_.homing_travel_speed() };
      auto const& homing_sensor{ pos_.homing_sensor() };
      if (!travel_speed.has_value() || !homing_sensor.has_value()) {
        return;
      }
      logger_.trace("Homing motor");
      if (homing_sensor->value().has_value() && homing_sensor->value().value()) {
        // todo move out of sensor and back to sensor
        auto const pos{ pos_.position() };
        logger_.info("Already at home, storing position: {}", pos);
        pos_.home(pos);
        return;
      }
      auto [order, ec1, ec2]{
        asio::experimental::make_parallel_group([&](auto token) { return run_at_speedratio(travel_speed.value(), token); },
                                                [&](auto token) { return homing_complete_.async_wait(token); })
            .async_wait(asio::experimental::wait_for_one(), bind_cancellation_slot(cancel_signal_.slot(), yield))
      };
      std::error_code err{ order[0] == 0 ? ec1 : ec2 };
      if (err != std::errc::operation_canceled) {
        [[maybe_unused]] std::error_code todo{ quick_stop(yield) };
        auto const pos{ pos_.position() };
        logger_.trace("Storing home position: {}", pos);
        pos_.home(pos);
      }
      if (err) {
        logger_.warn("Homing failed: {}", err.message());
      }
    });

    // returns { error_code, actual displacement }
    dbus_interface_->register_method(
        std::string{ method::convey_micrometre },
        [this](asio::yield_context yield, sdbusplus::message_t const& msg,
               micrometre_t travel) -> std::tuple<motor::errors::err_enum, micrometre_t> {
          using enum motor::errors::err_enum;
          if (!validate_peer(msg.get_sender())) {
            return std::make_tuple(permission_denied, 0L * micrometre_t::reference);
          }
          auto [err, actual_displacement]{ convey_micrometre(travel, bind_cancellation_slot(cancel_signal_.slot(), yield)) };
          return std::make_tuple(err, actual_displacement);
        });

    // returns { error_code, absolute position relative to home }
    dbus_interface_->register_method(
        std::string{ method::move_speedratio_micrometre },
        [this](asio::yield_context yield, sdbusplus::message_t const& msg, speedratio_t speedratio,
               micrometre_t placement) -> std::tuple<motor::errors::err_enum, micrometre_t> {
          return move(std::move(yield), msg, speedratio, placement);
        });

    // returns { error_code, absolute position relative to home }
    dbus_interface_->register_method(std::string{ method::move_micrometre },
                                     [this](asio::yield_context yield, sdbusplus::message_t const& msg,
                                            micrometre_t placement) -> std::tuple<motor::errors::err_enum, micrometre_t> {
                                       return move(std::move(yield), msg, config_speedratio_, placement);
                                     });

    dbus_interface_->register_property_r<std::string>(std::string{ connected_peer },
                                                      sdbusplus::vtable::property_::emits_change,
                                                      [this](const auto&) -> std::string { return peer_; });
    dbus_interface_->register_property_r<std::string>(std::string{ state_402 }, sdbusplus::vtable::property_::emits_change,
                                                      [this](const auto&) -> std::string {
                                                        std::string state{ cia_402::to_string(status_word_.parse_state()) };
                                                        return state;
                                                      });
    dbus_interface_->register_property_r<std::uint16_t>(std::string{ hmis }, sdbusplus::vtable::property_::emits_change,
                                                        [](const auto&) -> std::uint16_t { return 0; });

    dbus_interface_->initialize();
  }

  auto set_configured_speedratio(speedratio_t speedratio) {
    auto old_config_speedratio{ config_speedratio_ };
    config_speedratio_ = speedratio;
    // this is not correct, but this operation is non frequent and this simplifies the logic
    if (action == cia_402::transition_action::run && speed_ratio_ == old_config_speedratio) {
      speed_ratio_ = config_speedratio_;
    }
  }

  auto set_motor_nominal_freq(decifrequency nominal_motor_frequency) { motor_nominal_frequency_ = nominal_motor_frequency; }

  auto move(asio::yield_context yield, sdbusplus::message_t const& msg, speedratio_t speedratio, micrometre_t placement)
      -> std::tuple<motor::errors::err_enum, micrometre_t> {
    using enum motor::errors::err_enum;
    // Get our distance from the homing reference
    micrometre_t pos_from_home{ pos_.position_from_home().force_in(micrometre_t::reference) };
    if (!validate_peer(msg.get_sender())) {
      return std::make_tuple(permission_denied, pos_from_home);
    }
    cancel_pending_operation();
    if (!pos_.homing_enabled() || pos_.error() == motor_missing_home_reference) {
      logger_.trace("{}", motor_missing_home_reference);
      return std::make_tuple(motor_missing_home_reference, pos_from_home);
    }
    auto const resolution{ pos_.resolution() };
    logger_.trace("Target placement: {}, currently at: {}, with resolution: {}", placement, pos_from_home, resolution);
    if (placement + resolution >= pos_from_home && placement < pos_from_home + resolution) {
      logger_.trace("Currently within resolution of current position");
      return std::make_tuple(success, placement);
    }
    bool const is_positive{ pos_from_home < placement };
    // I thought about using absolute of speedratio, but if normal operation is negative than
    // it won't work, so it is better to making the user responsible to send correct sign.
    // Todo how can we document dbus API method calls?, generically.
    auto [order, ec1,
          ec2]{ asio::experimental::make_parallel_group(
                    [&](auto token) { return run_at_speedratio(is_positive ? speedratio : -speedratio, token); },
                    [&](auto token) { return pos_.notify_from_home(placement, token); })
                    .async_wait(asio::experimental::wait_for_one(), bind_cancellation_slot(cancel_signal_.slot(), yield)) };
    std::error_code err{ order[0] == 0 ? ec1 : ec2 };
    // Todo this stops quickly :-)
    // imagining 6 DOF robot arm, moving towards a specific radian in 3D space, it would depend on where the arm is going
    // so a single config variable for deceleration would not be sufficient, I propose to add it(deceleration time) to the
    // API call when needed implementing deceleration would propably be best to be controlled by this code not the atv
    // itself, meaning decrement given speedratio to 1% (using the given deceleration time) when 1% is reached next decrement
    // will quick_stop? or stop?
    if (err != std::errc::operation_canceled) {
      [[maybe_unused]] std::error_code todo{ quick_stop(bind_cancellation_slot(cancel_signal_.slot(), yield)) };
      pos_from_home = pos_.position_from_home().force_in(micrometre_t::reference);
      logger_.trace("{}, now at: {}, where target is: {}", is_positive ? "Moved" : "Moved back", pos_from_home, placement);
    }
    if (err) {
      return std::make_tuple(motor::motor_enum(err), pos_from_home);
    }
    return std::make_tuple(success, pos_from_home);
  }

  auto validate_peer(std::string_view incoming_peer) -> bool {
    if (incoming_peer != peer_) {
      logger_.warn("Peer rejected: {}", incoming_peer);
      return false;
    }
    return true;
  }

  auto run_at_speedratio(speedratio_t speedratio, asio::completion_token_for<void(std::error_code)> auto&& token) ->
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
    action = cia_402::transition_action::run;
    speed_ratio_ = speedratio;
    return run_blocker_.async_wait(token);
  }

  auto stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    action = cia_402::transition_action::stop;
    speed_ratio_ = 0 * mp_units::percent;
    return stop_complete_.async_wait(std::forward<decltype(token)>(token));
  }

  auto quick_stop(asio::completion_token_for<void(std::error_code)> auto&& token) ->
      typename asio::async_result<std::decay_t<decltype(token)>, void(std::error_code)>::return_type {
    action = cia_402::transition_action::quick_stop;
    speed_ratio_ = 0 * mp_units::percent;
    return stop_complete_.async_wait(std::forward<decltype(token)>(token));
  }

  bool cancel_pending_operation() {
    cancel_signal_.emit(asio::cancellation_type::all);
    return true;
  }

  // Set properties with new status values
  void update_status(const input_t& in) {
    status_word_ = in.status_word;
    pos_.freq_update(in.frequency);
    if (in.frequency == 0 * mp_units::si::hertz) {
      [[maybe_unused]] boost::system::error_code err{};
      stop_complete_.notify_all(err);
    }
    auto state = status_word_.parse_state();
    using enum motor::errors::err_enum;
    drive_error_ = {};
    if (cia_402::states_e::fault == state || cia_402::states_e::fault_reaction_active == state) {
      drive_error_ = motor_general_error;
    }
    if (drive_error_ != success && in.last_error == lft_e::cnf) {
      drive_error_ = motor_communication_fault;
    }
    if (drive_error_ != motor::errors::err_enum::success) {
      // big TODO propagate drive error to pending operation
      cancel_pending_operation();
    }
  }

  void on_homing_sensor(bool new_v) {
    logger_.trace("New homing sensor value: {}", new_v);
    if (new_v) {
      homing_complete_.notify_all();
    }
  }

  //
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
    if (speed_ratio_ < 1 * mp_units::percent && speed_ratio_ > -1 * mp_units::percent && run == action)
      return cia_402::transition(status_word_.parse_state(), none, allow_reset);
    return cia_402::transition(status_word_.parse_state(), action, allow_reset);
  }

  asio::io_context& ctx_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;  // todo is this needed, if so why, I am curious
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  asio::steady_timer timeout_{ ctx_ };
  std::string peer_{ "" };
  tfc::asio::condition_variable<asio::any_io_executor> run_blocker_{ ctx_.get_executor() };
  tfc::asio::condition_variable<asio::any_io_executor> stop_complete_{ ctx_.get_executor() };
  tfc::asio::condition_variable<asio::any_io_executor> homing_complete_{ ctx_.get_executor() };
  asio::cancellation_signal cancel_signal_{};

  // Motor control parameters
  cia_402::transition_action action{ cia_402::transition_action::none };
  speedratio_t speed_ratio_{ 0.0 * mp_units::percent };
  cia_402::status_word status_word_{};
  decifrequency motor_nominal_frequency_{};  // Indication if this is a 50Hz motor or 120Hz motor. That number has an effect
  // on dec and acc duration

  const uint16_t slave_id_;
  speedratio_t config_speedratio_{ 0.0 * mp_units::percent };
  motor::errors::err_enum drive_error_{};

  ipc_ruler::ipc_manager_client manager_;
  motor::positioner::positioner<> pos_;
  logger::logger logger_;

  /**
   * \brief has_peer
   * \return true if the dbus interface is being used to control the drive.
   */
  bool has_peer() { return peer_ != ""; }
};
}  // namespace tfc::ec::devices::schneider::atv320
