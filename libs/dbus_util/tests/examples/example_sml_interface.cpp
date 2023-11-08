#include <memory>
#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <boost/sml.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

#include <tfc/dbus/sml_interface.hpp>
#include <tfc/dbus/string_maker.hpp>

struct running {
  static constexpr std::string_view name{ "running" };
};
struct not_running {
  static constexpr std::string_view name{ "not_running" };
};

struct control_modes {
  auto operator()() {
    using boost::sml::event;
    using boost::sml::operator""_s;

    auto table = boost::sml::make_transition_table(*"not_running"_s + event<running> = "running"_s,
                                                   "running"_s + event<not_running> = "not_running"_s);
    return table;
  }
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);
  boost::asio::io_context ctx{};

  tfc::ipc_ruler::ipc_manager_client const ipc_client{ ctx };

  std::shared_ptr<sdbusplus::asio::dbus_interface> const interface {
    std::make_shared<sdbusplus::asio::dbus_interface>(ipc_client.connection(),
                                                      std::string{ tfc::dbus::sml::tags::path },
                                                      tfc::dbus::make_dbus_name("example_state_machine"))
  };

  tfc::dbus::sml::interface sml_interface {
    interface, "Log key"
  };  // optional log key
  // NOTE! interface struct requires to be passed by l-value like below, so the using code needs to store it like above

  using state_machine_t = boost::sml::sm<control_modes, boost::sml::logger<tfc::dbus::sml::interface> >;

  std::shared_ptr<state_machine_t> const state_machine{ std::make_shared<state_machine_t>(control_modes{}, sml_interface) };

  interface->initialize();

  state_machine->process_event(running{});

  ctx.run();

  return EXIT_SUCCESS;
}
