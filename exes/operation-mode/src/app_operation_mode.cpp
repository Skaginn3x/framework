#include "app_operation_mode.hpp"
#include "state_machine.hpp"

#include <string>

#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>

namespace tfc {

app_operation_mode::app_operation_mode(boost::asio::io_context& ctx)
    : dbus_{ std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system()) },
      dbus_object_server_{ std::make_unique<sdbusplus::asio::object_server>(dbus_) },
      dbus_interface_{ dbus_object_server_->add_interface(
          std::string{ operation::dbus::path.data(), operation::dbus::path.size() },
          std::string{ operation::dbus::name.data(), operation::dbus::name.size() }) },
      state_machine_{ std::make_unique<operation::state_machine>(ctx) }, logger_{ "app_operation_mode" } {
  dbus_interface_->register_signal<operation::update_message>(
      std::string(operation::dbus::signal::update.data(), operation::dbus::signal::update.size()));

  dbus_interface_->register_method("set_mode", [this](tfc::operation::mode_e new_mode) {
    if (auto const err_code{ state_machine_->set_mode(new_mode) }) {
      logger_.info("Unable to set to state: '{}', error: '{}'", mode_e_str(new_mode), err_code.message());
      return;
    }
    logger_.trace("set_mode to state: '{}'", mode_e_str(new_mode));
  });

  state_machine_->on_new_mode([this](operation::new_mode new_mode, operation::old_mode old_mode) {
    logger_.trace("sending update signal from state: '{}' to state: '{}'", mode_e_str(old_mode), mode_e_str(new_mode));
    auto message{ dbus_interface_->new_signal(operation::dbus::signal::update.data()) };
    message.append(operation::update_message{ .new_mode = new_mode, .old_mode = old_mode });
    message.signal_send();
  });

  dbus_->request_name(operation::dbus::name.data());

  dbus_interface_->initialize();
}

}  // namespace tfc
