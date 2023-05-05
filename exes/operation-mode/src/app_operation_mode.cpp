#include "app_operation_mode.hpp"
#include "state_machine.hpp"

#include <string>

#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

namespace tfc {

// todo move to dbus_utils
static auto open_sd_bus() {
  sd_bus* bus = nullptr;
  if (sd_bus_open(&bus) < 0) {
    throw std::runtime_error(std::string("Unable to open sd-bus, error: ") + strerror(errno));
  }
  return bus;
}

app_operation_mode::app_operation_mode(boost::asio::io_context& ctx)
    : dbus_{ std::make_shared<sdbusplus::asio::connection>(ctx, open_sd_bus()) },
      dbus_object_server_{
        std::unique_ptr<sdbusplus::asio::object_server, std::function<void(sdbusplus::asio::object_server*)>>(
            new sdbusplus::asio::object_server(dbus_),
            [](sdbusplus::asio::object_server* ptr) { delete ptr; })
      },
      dbus_interface_{ dbus_object_server_->add_interface(
          std::string{ operation::dbus::path.data(), operation::dbus::path.size() },
          std::string{ operation::dbus::name.data(), operation::dbus::name.size() }) },
      state_machine_{ std::unique_ptr<operation::state_machine, std::function<void(operation::state_machine*)>>{
          new operation::state_machine(), [](operation::state_machine* ptr) { delete ptr; } } } {
  dbus_interface_->register_signal<operation::update_message>(
      std::string(operation::dbus::signal::update.data(), operation::dbus::signal::update.size()));

  dbus_interface_->register_method("set_mode", [this](tfc::operation::mode_e new_mode){
    auto const old_mode = state_machine_->get_mode();
    state_machine_->try_set_mode(new_mode); // todo use return value throw dbus error?
    auto message{ dbus_interface_->new_signal(operation::dbus::signal::update.data()) };
    message.append(operation::update_message{ .new_mode=new_mode, .old_mode=old_mode });
    message.signal_send();
  });

  dbus_->request_name(operation::dbus::name.data());

  dbus_interface_->initialize();
}

}  // namespace tfc
