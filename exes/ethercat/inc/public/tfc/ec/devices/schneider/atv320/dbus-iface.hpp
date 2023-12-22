/*
  An interface to allow programs rpc control of the ATV320 drive.
  This is a dbus interface and is more "in depth" then the simple
  ipc-run and speed interface.
*/
#pragma once

#include <iostream>  //TODO: Remove
#include <string>

#include <boost/asio.hpp>
#include <tfc/cia/402.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ec/devices/schneider/atv320/pdo.hpp>
#include <tfc/ec/devices/schneider/atv320/speedratio.hpp>

namespace tfc::ec::devices::schneider::atv320 {

namespace asio = boost::asio;

// Handy commands
// sudo busctl introspect com.skaginn3x.atv320 /com/skaginn3x/atvmotor
//
struct dbus_iface {
  // Properties
  static constexpr std::string_view connected_peer{ "connected_peer" };
  static constexpr std::string_view frequency{ "frequency" };
  static constexpr std::string_view state_402{ "state_402" };
  static constexpr std::string_view hmis{ "hmis" };

  dbus_iface(std::shared_ptr<sdbusplus::asio::connection> connection, const uint16_t slave_id)
      : ctx_(connection->get_io_context()), timeout_(ctx_), slave_id_{ slave_id } {
    sd_bus* bus = nullptr;
    if (sd_bus_open_system(&bus) < 0) {
      throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
    }
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection, false);
    dbus_interface_ =
        object_server_->add_unique_interface(fmt::format("/com/skaginn3x/atvmotor{}", slave_id), "com.skaginn3x.atvmotor");
    dbus_interface_->register_method("ping", [this](const sdbusplus::message_t& msg) -> bool {
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
          speed_ratio_ = 0.0;
          peer_ = "";
          dbus_interface_->signal_property(std::string{ connected_peer });
        });
        return true;
      } else {
        // Peer rejected
        return false;
      }
    });

    dbus_interface_->register_method("run_at_speedratio",
                                     [this](const sdbusplus::message_t& msg, const double& speedratio) -> bool {
                                       std::string incoming_peer = msg.get_sender();
                                       if (incoming_peer != peer_) {
                                         std::cerr << "Peer rejected" << incoming_peer << std::endl;  // TODO: Remove
                                         return false;
                                       }
                                       if (speedratio < -100 || speedratio > 100) {
                                         return false;
                                       }
                                       std::cout << "run_motor " << speedratio << std::endl;  // TODO: Remove
                                       quick_stop_ = false;
                                       op_enable_ = true;
                                       speed_ratio_ = speedratio;
                                       return true;
                                     });

    dbus_interface_->register_method("stop", [this](const sdbusplus::message_t& msg) -> bool {
      std::string incoming_peer = msg.get_sender();
      if (incoming_peer != peer_) {
        std::cerr << "Peer rejected" << incoming_peer << std::endl;  // TODO: Remove
        return false;
      }
      quick_stop_ = false;
      op_enable_ = false;
      speed_ratio_ = 0;
      return true;
    });

    dbus_interface_->register_method("quick_stop", [this](const sdbusplus::message_t& msg) -> bool {
      std::string incoming_peer = msg.get_sender();
      if (incoming_peer != peer_) {
        std::cerr << "Peer rejected" << incoming_peer << std::endl;  // TODO: Remove
        return false;
      }
      quick_stop_ = true;
      op_enable_ = true;
      speed_ratio_ = 0;
      return true;
    });

    dbus_interface_->register_property_r<std::string>(std::string{ connected_peer },
                                                      sdbusplus::vtable::property_::emits_change,
                                                      [this](const auto&) -> std::string { return peer_; });
    dbus_interface_->register_property_r<std::uint16_t>(std::string{ hmis }, sdbusplus::vtable::property_::emits_change,
                                                        [](const auto&) -> std::uint16_t { return 0; });

    dbus_interface_->initialize();
  }
  // Set properties with new status values
  void update_status(const input_t& in) { status_word_ = in.status_word; }
  //
  mp_units::quantity<mp_units::percent, double> speed_ratio() { return speed_ratio_ * mp_units::percent; }
  cia_402::control_word ctrl() { return cia_402::transition(status_word_.parse_state(), op_enable_, quick_stop_, false); }
  asio::io_context& ctx_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  asio::steady_timer timeout_;
  std::string peer_{ "" };

  // Motor control parameters
  bool quick_stop_{ false };
  bool op_enable_{ false };
  double speed_ratio_{ 0.0 };
  cia_402::status_word status_word_{};

  const uint16_t slave_id_;

  /**
   * \brief has_peer
   * \return true if the dbus interface is being used to control the drive.
   */
  bool has_peer() { return peer_ != ""; }
};
}  // namespace tfc::ec::devices::schneider::atv320
