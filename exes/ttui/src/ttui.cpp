#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
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

#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

namespace po = boost::program_options;

auto left_menu(const std::vector<std::string>* entries, int* selected, const ftxui::Component& right_menu) -> ftxui::Component {
  auto option = ftxui::MenuOption::Vertical();

  option.on_enter = [selected, right_menu] {
    right_menu->TakeFocus();
  };

  return ftxui::Menu(entries, selected, option);

};

auto right_menu(const std::vector<std::string>* entries, int* selected) -> ftxui::Component {
  const auto option = ftxui::MenuOption::Vertical();
  // fill remaining horizontal space
  return ftxui::Menu(entries, selected, option);

};

std::vector<std::string> findDBusServicesStartingWith(const std::string& prefix) {

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

  const std::string prefix = "com.skaginn3x";
  auto services = findDBusServicesStartingWith(prefix);

  ftxui::Component right_menuz = right_menu(&right_menu_entries, &right_menu_selected);
  ftxui::Component left_menuz = left_menu(&services, &left_menu_selected, right_menuz);


  ftxui::Component container = ftxui::Container::Horizontal({
      left_menuz,
      right_menuz
  });

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

  screen.Loop(renderer);
};
