#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/loop.hpp"                // for ScreenInteractive
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive

#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <fmt/core.h>
#include <mp-units/format.h>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <ftxui/component/component.hpp>
#include <iostream>

#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/bus/match.hpp>

#include <tfc/dbus/match_rules.hpp>
#include <tfc/dbus/sd_bus.hpp>

#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;

using variant_t = std::variant<bool, int64_t, uint64_t, double, std::string, std::vector<std::string>>;

std::vector<std::string> noticed_interfaces{};
std::vector<std::variant<bool, uint64_t>> values{};
std::vector<std::string> entries;
int global_left_selected = 0;

auto left_menu(const std::vector<std::string>*, int* selected, const ftxui::Component& right_menu) -> ftxui::Component {
  auto option = ftxui::MenuOption::Vertical();

  option.on_enter = [selected, right_menu] {
    global_left_selected = *selected;
    std::string current_value = std::visit([](auto& value) { return std::to_string(value); }, values[*selected]);
    entries = { current_value };
    right_menu->TakeFocus();
  };

  return ftxui::Menu(&noticed_interfaces, selected, option);
};
auto right_menu(int* selected) -> ftxui::Component {
  const auto option = ftxui::MenuOption::Vertical();
  // fill remaining horizontal space
  return ftxui::Menu(&entries, selected, option);
};

// GET DEMM DBUS DATA
auto match_callback(sdbusplus::message_t& msg, ftxui::Component left, ftxui::Component right ) -> void {
  // std::cout << msg.get_member() << std::endl;
  // std::cout << msg.get_signature() << std::endl;
  // std::cout << msg.get_sender() << std::endl;
  // std::cout << msg.get_interface() << std::endl;
  // std::cout << msg.get_path() << std::endl;
  if (msg.get_member() && std::string(msg.get_member()) == "PropertiesChanged") {
    std::tuple<std::string, std::vector<std::pair<std::string, std::variant<bool, uint64_t>>>, std::vector<std::string>>
        container{};
    if (sdbusplus::utility::read_into_tuple(container, msg)) {
      std::string const& interface = std::get<0>(container);
      auto position = std::find(noticed_interfaces.begin(), noticed_interfaces.end(), interface);
      auto value = std::get<1>(container)[0].second;
      if (position == noticed_interfaces.end()) {
        left->Render();
        noticed_interfaces.emplace_back(interface);
        values.emplace_back();
      } else {
        auto index = position - noticed_interfaces.begin();
        values[index] = value;

        std::string current_value = std::visit([](auto& value) { return std::to_string(value); }, values[index]);
        entries = { current_value };
        if (index == global_left_selected) {
          right->Render();
        }
      }
      // std::visit([&interface](auto& value) { std::cout << interface << ": " << value << std::endl; },
      // std::get<1>(container)[0].second);
    }
  }
}

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };

  std::string signal{};
  std::string slot{};
  std::vector<std::string> connect;
  bool list_signals{};
  bool list_slots{};

  description.add_options()("signal", po::value<std::string>(&signal), "IPC signal channel (output)")(
      "slot", po::value<std::string>(&slot), "IPC slot channel (input)")(
      "connect,c", po::value<std::vector<std::string>>(&connect)->multitoken(), "Listen to these slots")(
      "list-signals", po::bool_switch(&list_signals), "List all available IPC signals")(
      "list-slots", po::bool_switch(&list_slots), "List all available IPC slots");
  tfc::base::init(argc, argv, description);

  auto screen = ftxui::ScreenInteractive::TerminalOutput();

  std::vector<std::string> left_menu_entries = {
    "0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%",
  };
  std::vector<std::string> right_menu_entries = {
    "0%", "1%", "2%", "3%", "4%", "5%", "6%", "7%", "8%", "9%", "10%",
  };

  auto menu_option = ftxui::MenuOption();
  menu_option.on_enter = screen.ExitLoopClosure();

  int left_menu_selected = 0;
  int right_menu_selected = 0;

  std::vector<std::string> services = { "hi" };
  ftxui::Component right_menuz = right_menu(&right_menu_selected);
  ftxui::Component left_menuz = left_menu(&services, &left_menu_selected, right_menuz);

  ftxui::Component container = ftxui::Container::Horizontal({ left_menuz, right_menuz });

  auto renderer = Renderer(container, [&] {
    return ftxui::vbox({
               // -------- Top panel --------------
               ftxui::hbox({
                   // -------- Left Menu --------------
                   ftxui::vbox({
                       ftxui::hcenter(ftxui::bold(ftxui::text("Percentage by 10%"))),
                       ftxui::separator(),
                       left_menuz->Render(),
                   }),
                   ftxui::separator(),
                   // -------- Right Menu --------------
                   ftxui::vbox({
                       ftxui::hcenter(ftxui::bold(ftxui::text("Percentage by 1%"))),
                       ftxui::separator(),
                       right_menuz->Render(),
                   }) | ftxui::flex,
                   ftxui::separator(),
               }),
           }) |
           ftxui::border | ftxui::flex;
  });
  ftxui::Loop loop(&screen, renderer);

  asio::io_context ctx{};
  std::shared_ptr<sdbusplus::asio::connection> connection =
      std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system_mon());
  // Attach callback
  auto match = std::make_unique<sdbusplus::bus::match::match>(*connection, "", std::bind(match_callback, std::placeholders::_1, left_menuz, right_menuz));
  // Become monitor method
  auto mc = connection->new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus.Monitoring",
                                        "BecomeMonitor");
  mc.append<std::vector<std::string>, uint32_t>({ "path=/com/skaginn3x/Signals" }, 0);
  auto reply = connection->call(mc);
  if (reply.is_method_error()) {
    if (reply.get_error() != nullptr)
      throw std::runtime_error(reply.get_error()->message);
    throw std::runtime_error("Unknown error");
  }

  // Run the loop until the screen quits
  while (!loop.HasQuitted()) {
    loop.RunOnce();
    ctx.run_for(std::chrono::milliseconds(10));
  }
};
