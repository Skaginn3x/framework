#include <memory>
#include <string_view>

#include <boost/asio.hpp>
#include <boost/sml.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <tfc/dbus/sml_interface.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/progbase.hpp>

struct run {
  static constexpr std::string_view name{ "run" };
};
struct stop {
  static constexpr std::string_view name{ "stop" };
};

struct control_modes {
  auto operator()() {
    using boost::sml::event;
    using boost::sml::operator""_s;

    auto table = boost::sml::make_transition_table(*"not_running"_s + event<run> = "running"_s,
                                                   "running"_s + event<stop> = "not_running"_s);
    return table;
  }
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);
  boost::asio::io_context ctx{};

  // Raw dbus connection, ipc_client also has a dbus connection which can be used through ipc_client.connection()
  auto const dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };

  tfc::dbus::sml::interface sml_interface {
    dbus, "unique_key"
  };

  using state_machine_t = boost::sml::sm<control_modes, boost::sml::logger<tfc::dbus::sml::interface> >;

  // NOTE! interface struct requires to be passed by l-value like below, so the using code needs to store it like above
  std::shared_ptr<state_machine_t> const state_machine{ std::make_shared<state_machine_t>(control_modes{}, sml_interface) };

  state_machine->process_event(run{});

  ctx.run();

  return EXIT_SUCCESS;
}
