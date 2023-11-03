#include <memory>

#include <boost/asio.hpp>
#include <boost/sml.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/dbus/sml_interface.hpp>

#include <iostream>

struct state_machine {
  auto operator()() {
    using boost::sml::operator""_s;
    using boost::sml::operator""_e;
    return boost::sml::make_transition_table(*"init"_s + "set_stopped"_e = "stopped"_s);
  }
};

int main() {
  boost::asio::io_context ctx{};

  std::shared_ptr<sdbusplus::asio::connection> bus{ std::make_shared<sdbusplus::asio::connection>(ctx) };

  std::shared_ptr<sdbusplus::asio::dbus_interface> interface {
    std::make_shared<sdbusplus::asio::dbus_interface>(bus,
                                                      std::string{ tfc::dbus::sml::tags::path },
                                                      tfc::dbus::make_dbus_name("example_state_machine"))
  };

  interface->initialize();

  tfc::dbus::sml::interface sml_interface {
    interface, "example_state_machine"
  };

  using state_machine_t = boost::sml::sm<state_machine, boost::sml::logger<tfc::dbus::sml::interface> >;

  std::shared_ptr<state_machine_t> combined_state_machine_{ std::make_shared<state_machine_t>(state_machine{},
                                                                                              sml_interface) };

  ctx.run();

  return EXIT_SUCCESS;
}
