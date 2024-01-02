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

// Handy commands
// sudo busctl introspect com.skaginn3x.atv320 /com/skaginn3x/atvmotor
//
template <typename manager_client_t>
struct dbus_iface {
  // Properties
  static constexpr std::string_view connected_peer{ "connected_peer" };
  static constexpr std::string_view frequency{ "frequency" };
  static constexpr std::string_view state_402{ "state_402" };
  static constexpr std::string_view hmis{ "hmis" };  // todo change to more readable form
  static constexpr std::string_view impl_name{ "atv320" };

  dbus_iface(std::shared_ptr<sdbusplus::asio::connection> connection,
             const uint16_t slave_id,
             manager_client_t& manager_client)
      : ctx_(connection->get_io_context()), slave_id_{ slave_id },
        pos_{ ctx_, manager_client, fmt::format("{}_{}", impl_name, slave_id_),
              std::bind_front(&dbus_iface::on_homing_sensor, this) },
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
        timeout_.expires_after(std::chrono::milliseconds(750));
        timeout_.async_wait([this](std::error_code err) {
          if (err)
            return;  // The timer was canceled or deconstructed.
          op_enable_ = false;
          quick_stop_ = false;
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
        std::string{ method::run_at_speedratio },
        [this](const sdbusplus::message_t& msg, mp_units::quantity<mp_units::percent, double> speedratio) -> bool {
          return validate_peer(msg.get_sender()) && run_at_speedratio(speedratio);
        });
    dbus_interface_->register_method(
        std::string{ method::notify_after_nanometre },
        [this](asio::yield_context yield, const sdbusplus::message_t& msg,
               const mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t>& distance) {
          std::string incoming_peer = msg.get_sender();
          pos_.notify_after(distance, yield);
        });

    dbus_interface_->register_method(std::string{ method::stop }, [this](const sdbusplus::message_t& msg) -> bool {
      return validate_peer(msg.get_sender()) && stop();
    });

    dbus_interface_->register_method(std::string{ method::quick_stop }, [this](const sdbusplus::message_t& msg) -> bool {
      return validate_peer(msg.get_sender()) && stop();  // todo quick stop
    });

    dbus_interface_->register_method(std::string{ method::do_homing }, [this](asio::yield_context yield,
                                                                              const sdbusplus::message_t& msg) {
      if (!validate_peer(msg.get_sender())) {
        return;
      }
      auto const& travel_speed{ pos_.homing_travel_speed() };
      auto const& homing_sensor{ pos_.homing_sensor() };
      if (!travel_speed.has_value() || !homing_sensor.has_value()) {
        return;
      }
      logger_.trace("Homing motor");
      if (homing_sensor->value().has_value() && homing_sensor->value().value()) {
        auto const pos{ pos_.position() };
        logger_.info("Already at home, storing position: {}", pos);
        pos_.home(pos);
        return;
      }
      run_at_speedratio(travel_speed.value());
      std::error_code err{ homing_complete_.async_wait(yield) };  // todo if other action is executed need to cancel the wait
      auto const pos{ pos_.position() };
      stop();
      if (err) {
        logger_.warn("Homing failed: {}", err.message());
        return;
      }
      logger_.trace("Storing home position: {}", pos);
      pos_.home(pos);
    });

    dbus_interface_->register_property_r<std::string>(std::string{ connected_peer },
                                                      sdbusplus::vtable::property_::emits_change,
                                                      [this](const auto&) -> std::string { return peer_; });
    dbus_interface_->register_property_r<std::uint16_t>(std::string{ hmis }, sdbusplus::vtable::property_::emits_change,
                                                        [](const auto&) -> std::uint16_t { return 0; });

    dbus_interface_->initialize();
  }

  auto validate_peer(std::string_view incoming_peer) -> bool {
    if (incoming_peer != peer_) {
      logger_.warn("Peer rejected: {}", incoming_peer);
      return false;
    }
    return true;
  }
  auto run_at_speedratio(mp_units::quantity<mp_units::percent, double> speedratio) -> bool {
    if (speedratio < -100 * mp_units::percent || speedratio > 100 * mp_units::percent) {
      return false;
    }
    logger_.trace("Run motor at speedratio: {}", speedratio);
    quick_stop_ = false;
    op_enable_ = true;
    speed_ratio_ = speedratio;
    return true;
  }
  bool stop() {
    quick_stop_ = true;
    op_enable_ = true;
    speed_ratio_ = 0 * mp_units::percent;
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
  mp_units::quantity<mp_units::percent, double> speed_ratio() { return speed_ratio_ * mp_units::percent; }
  cia_402::control_word ctrl() { return cia_402::transition(status_word_.parse_state(), op_enable_, quick_stop_, false); }
  asio::io_context& ctx_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;  // todo is this needed, if so why, I am curious
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  asio::steady_timer timeout_{ ctx_ };
  std::string peer_{ "" };
  tfc::asio::condition_variable<asio::any_io_executor> homing_complete_{ ctx_.get_executor() };

  // Motor control parameters
  bool quick_stop_{ false };
  bool op_enable_{ false };
  mp_units::quantity<mp_units::percent, double> speed_ratio_{ 0.0 * mp_units::percent };
  cia_402::status_word status_word_{};

  const uint16_t slave_id_;

  tfc::motor::positioner::positioner<> pos_;
  tfc::logger::logger logger_;

  /**
   * \brief has_peer
   * \return true if the dbus interface is being used to control the drive.
   */
  bool has_peer() { return peer_ != ""; }
};
}  // namespace tfc::ec::devices::schneider::atv320
