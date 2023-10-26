#include <boost/asio.hpp>
#include <boost/sml.hpp>
#include <memory>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <tfc/dbus/sml_interface.hpp>
struct state_machine {
  auto operator()() {
    using boost::sml::operator""_s;
    using boost::sml::operator""_e;
    return boost::sml::make_transition_table(*"init"_s + "set_stopped"_e = "stopped"_s);
  }
};
int main() {
  boost::asio::io_context ctx{};
  auto bus = std::make_shared<sdbusplus::asio::connection>(ctx);
  auto interface =
      std::make_shared<sdbusplus::asio::dbus_interface>(bus, std::string{ tfc::dbus::sml::tags::path }, "StateMachineName");
  tfc::dbus::sml::interface sml_interface {
    interface, "Log key"
  };  // optional log key
  // NOTE! interface struct requires to be passed by l-value like below, so the using code needs to store it like above
  boost::sml::sm<state_machine, boost::sml::logger<tfc::dbus::sml::interface>> my_sm{ sml_interface };
  return EXIT_SUCCESS;
}
