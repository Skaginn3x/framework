/*
  An interface to allow programs rpc control of the ATV320 drive.
  This is a dbus interface and is more "in depth" then the simple
  ipc-run and speed interface.
*/
#pragma once

#include <iostream> //TODO: Remove
#include <string>

#include <boost/asio.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/ec/devices/schneider/atv320/pdo.hpp>
#include <tfc/cia/402.hpp>

namespace tfc::ec::devices::schneider::atv320 {

namespace asio = boost::asio;

// Handy commands
// sudo busctl introspect com.skaginn3x.atv320 /com/skaginn3x/atvmotor
//
struct dbus_iface {
  static constexpr std::string connected_peer = "connected_peer";
  dbus_iface(asio::io_context& ctx) : ctx_(ctx), timeout_(ctx_) {
    sd_bus* bus = nullptr;
    if (sd_bus_open_system(&bus) < 0) {
      throw std::runtime_error(std::string{ "Unable to open sd-bus, error: " } + strerror(errno));
    }
    connection_ = std::make_shared<sdbusplus::asio::connection>(ctx, bus);
    connection_->request_name("com.skaginn3x.atv320");
    object_server_ = std::make_unique<sdbusplus::asio::object_server>(connection_, false);
    dbus_interface_ = object_server_->add_unique_interface("/com/skaginn3x/atvmotor", "com.skaginn3x.atvmotor");
    dbus_interface_->register_method("ping", [this](const sdbusplus::message_t& msg) -> bool {
      std::string incoming_peer = msg.get_sender();
      // This is the same peer or we have no peer
      bool new_peer = incoming_peer != peer_;
      if (incoming_peer == peer_ || (new_peer && peer_ == "")) {
        if (new_peer) {
          peer_ = incoming_peer;
          dbus_interface_->signal_property(connected_peer);
        }
        timeout_.cancel();
        timeout_.expires_after(std::chrono::milliseconds(750));  // TODO: make this 750ms, send pings every 300ms
        timeout_.async_wait([this](std::error_code err) {
          if (err)
            return;                             // The timer was canceled or deconstructed.
          std::cout << "TIMEOUT" << std::endl;  // TODO: Remove
          peer_ = "";
          dbus_interface_->signal_property(connected_peer);
        });
        return true;
      } else {
        std::cerr << "Peer rejected" << std::endl;  // TODO: Remove
        return false;
      }
    });

    dbus_interface_->register_property_r<std::string>(connected_peer, sdbusplus::vtable::property_::emits_change,
                                                      [this](const auto&) -> std::string { return peer_; });

    dbus_interface_->initialize();
  }
  // Set properties with new status values
  void update_status(const input_t&) {
  }
  //
  decifrequency freq() {
    return 0 * dHz;
  }
  cia_402::control_word ctrl() {
    return cia_402::control_word{};
  }
  asio::io_context& ctx_;
  std::shared_ptr<sdbusplus::asio::connection> connection_;
  std::unique_ptr<sdbusplus::asio::object_server> object_server_;
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  asio::steady_timer timeout_;
  std::string peer_{ "" };

  /**
   * \brief has_peer
   * \return true if the dbus interface is being used to control the drive.
   */
  bool has_peer() { return peer_ != ""; }

};
}  // namespace tfc::ec::devices::schneider::atv320