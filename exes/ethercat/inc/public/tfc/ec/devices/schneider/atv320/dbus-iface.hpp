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

// Handy commands
// sudo busctl introspect com.skaginn3x.atv320 /com/skaginn3x/atvmotor
//
struct dbus_iface {
  // Properties
  static constexpr std::string_view connected_peer{ "connected_peer" };
  static constexpr std::string_view frequency{ "frequency" };
  static constexpr std::string_view state_402{ "state_402" };
  static constexpr std::string_view hmis{ "hmis" };  // todo change to more readable form
  static constexpr std::string_view impl_name{ "atv320" };

  dbus_iface(std::shared_ptr<sdbusplus::asio::connection> connection, const uint16_t slave_id)
      : ctx_(connection->get_io_context()), slave_id_{ slave_id },
        pos_{ connection, fmt::format("{}_{}", impl_name, slave_id_), std::bind_front(&dbus_iface::on_homing_sensor, this) },
        logger_(fmt::format("{}_{}", impl_name, slave_id_)) {
    sd_bus* bus = nullptr;
    if (sd_bus_open_system(&bus) < 0) {
      throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
    }
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

    dbus_interface_->register_method(
        std::string{ method::run_at_speedratio }, [this](const sdbusplus::message_t& msg, speedratio_t speedratio) -> bool {
          return validate_peer(msg.get_sender()) && cancel_pending_operation() && run_at_speedratio(speedratio);
        });
    dbus_interface_->register_method(
        std::string{ method::notify_after_micrometre },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg, micrometre_t distance) {
          std::string incoming_peer = msg.get_sender();
          pos_.notify_after(distance, yield);
        });

    dbus_interface_->register_method(std::string{ method::stop }, [this](const sdbusplus::message_t& msg) -> bool {
      return validate_peer(msg.get_sender()) && cancel_pending_operation() && stop();
    });

    dbus_interface_->register_method(std::string{ method::quick_stop }, [this](const sdbusplus::message_t& msg) -> bool {
      return validate_peer(msg.get_sender()) && cancel_pending_operation() && quick_stop();  // todo quick stop
    });

    dbus_interface_->register_method(
        std::string{ method::do_homing }, [this](asio::yield_context yield, const sdbusplus::message_t& msg) {
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
          run_at_speedratio(travel_speed.value());
          std::error_code err{ homing_complete_.async_wait(bind_cancellation_slot(cancel_signal_.slot(), yield)) };
          if (err != std::errc::operation_canceled) {
            quick_stop();
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
               micrometre_t displacement) -> std::tuple<motor::errors::err_enum, micrometre_t> {
          using enum motor::errors::err_enum;
          auto const pos{ pos_.position() };
          logger_.trace("Target displacement: {}, current position: {}", displacement, pos);
          if (!validate_peer(msg.get_sender())) {
            return std::make_tuple(permission_denied, 0L * micrometre_t::reference);
          }
          if (displacement == 0L * micrometre_t::reference) {
            return std::make_tuple(success, 0L * micrometre_t::reference);
          }
          cancel_pending_operation();
          bool const is_positive{ displacement > 0L * micrometre_t::reference };
          if (!run_at_speedratio(is_positive ? config_speedratio_ : -config_speedratio_)) {
            return std::make_tuple(speedratio_out_of_range, 0L * micrometre_t::reference);
          }
          std::error_code err{ pos_.notify_after(displacement, bind_cancellation_slot(cancel_signal_.slot(), yield)) };
          auto const actual_displacement{ is_positive ? (pos_.position() - pos).force_in(micrometre_t::reference)
                                                      : -(pos - pos_.position()).force_in(micrometre_t::reference) };
          // This is only called if another invocation has taken control of the motor
          // stopping the motor now would be counter productive as somebody is using it.
          if (err != std::errc::operation_canceled) {
            // Todo this stops quickly :-)
            quick_stop();
          }
          if (err) {
            logger_.warn("Convey failed: {}", err.message());
            return std::make_tuple(motor::motor_error(err), actual_displacement);
          }
          logger_.trace("Actual displacement: {}, where target was: {}", actual_displacement, displacement);
          return std::make_tuple(success, actual_displacement);
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

  auto move(asio::yield_context yield, sdbusplus::message_t const& msg, speedratio_t speedratio, micrometre_t placement)
      -> std::tuple<motor::errors::err_enum, micrometre_t> {
    using enum motor::errors::err_enum;
    // Get our distance from the homing reference
    micrometre_t pos_from_home{ pos_.position_from_home().force_in(micrometre_t::reference) };
    if (!validate_peer(msg.get_sender())) {
      return std::make_tuple(permission_denied, pos_from_home);
    }
    logger_.trace("Target placement: {}, currently at: {}", placement, pos_from_home);
    cancel_pending_operation();
    if (!pos_.homing_enabled() || pos_.error() == motor_missing_home_reference) {
      logger_.trace("{}", motor_missing_home_reference);
      return std::make_tuple(motor_missing_home_reference, pos_from_home);
    }
    auto const resolution{ pos_.resolution() };
    if (placement + resolution >= pos_from_home || placement < pos_from_home + resolution) {
      logger_.trace("Currently within resolution of current position");
      return std::make_tuple(success, placement);
    }
    bool const is_positive{ pos_from_home < placement };
    // I thought about using absolute of speedratio, but if normal operation is negative than
    // it won't work, so it is better to making the user responsible to send correct sign.
    // Todo how can we document dbus API method calls?, generically.
    if (!run_at_speedratio(is_positive ? speedratio : -speedratio)) {
      return std::make_tuple(speedratio_out_of_range, pos_from_home);
    }
    std::error_code err{ pos_.notify_from_home(placement, bind_cancellation_slot(cancel_signal_.slot(), yield)) };
    // Todo this stops quickly :-)
    // imagining 6 DOF robot arm, moving towards a specific radian in 3D space, it would depend on where the arm is going
    // so a single config variable for deceleration would not be sufficient, I propose to add it(deceleration time) to the
    // API call when needed implementing deceleration would propably be best to be controlled by this code not the atv
    // itself, meaning decrement given speedratio to 1% (using the given deceleration time) when 1% is reached next decrement
    // will quick_stop? or stop?
    if (err != std::errc::operation_canceled) {
      quick_stop();
      pos_from_home = pos_.position_from_home().force_in(micrometre_t::reference);
      logger_.trace("{}, now at: {}, where target is: {}", is_positive ? "Moved" : "Moved back", pos_from_home, placement);
    }
    if (err) {
      return std::make_tuple(motor::motor_error(err), pos_from_home);
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
  auto run_at_speedratio(speedratio_t speedratio) -> bool {
    if (speedratio < -100 * mp_units::percent || speedratio > 100 * mp_units::percent) {
      logger_.trace("Speedratio not within range [-100,100], value: {}", speedratio);
      return false;
    }
    logger_.trace("Run motor at speedratio: {}", speedratio);
    action = cia_402::transition_action::run;
    speed_ratio_ = speedratio;
    return true;
  }
  bool stop() {
    action = cia_402::transition_action::stop;
    speed_ratio_ = 0 * mp_units::percent;
    return true;
  }
  bool quick_stop() {
    action = cia_402::transition_action::quick_stop;
    speed_ratio_ = 0 * mp_units::percent;
    return true;
  }
  bool cancel_pending_operation() {
    cancel_signal_.emit(asio::cancellation_type::all);
    return true;
  }
  // Set properties with new status values
  void update_status(const input_t& in) {
    status_word_ = in.status_word;
    pos_.freq_update(in.frequency);
  }
  void on_homing_sensor(bool new_v) {
    logger_.trace("New homing sensor value: {}", new_v);
    if (new_v) {
      homing_complete_.notify_all();
    }
  }
  //
  speedratio_t speed_ratio() { return speed_ratio_; }
  cia_402::control_word ctrl(bool allow_reset) {
    if (speed_ratio_ < 1 * mp_units::percent && speed_ratio_ > -1 * mp_units::percent )
      return cia_402::transition(status_word_.parse_state(), cia_402::transition_action::none, allow_reset);
    return cia_402::transition(status_word_.parse_state(), action, allow_reset);
  }
  asio::io_context& ctx_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;  // todo is this needed, if so why, I am curious
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  asio::steady_timer timeout_{ ctx_ };
  std::string peer_{ "" };
  tfc::asio::condition_variable<asio::any_io_executor> homing_complete_{ ctx_.get_executor() };
  asio::cancellation_signal cancel_signal_{};

  // Motor control parameters
  cia_402::transition_action action{ cia_402::transition_action::none };
  speedratio_t speed_ratio_{ 0.0 * mp_units::percent };
  cia_402::status_word status_word_{};

  const uint16_t slave_id_;
  speedratio_t config_speedratio_{ 0.0 * mp_units::percent };

  tfc::motor::positioner::positioner<> pos_;
  tfc::logger::logger logger_;

  /**
   * \brief has_peer
   * \return true if the dbus interface is being used to control the drive.
   */
  bool has_peer() { return peer_ != ""; }
};
}  // namespace tfc::ec::devices::schneider::atv320
