#include "dbus_screen_manager.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus/match.hpp>
#include <tfc/dbus/sd_bus.hpp>

DBusScreenManager::DBusScreenManager() : screen(ftxui::ScreenInteractive::TerminalOutput()) {}

void DBusScreenManager::Run() {
  SetupComponents();
  SetupDBusConnection();
  ftxui::Loop loop(&screen, renderer);
  while (!loop.HasQuitted()) {
    loop.RunOnce();
    ctx.run_for(std::chrono::milliseconds(10));
  }
}

void DBusScreenManager::SetupComponents() {
  right_menuz = right_menu();
  left_menuz = left_menu();
  container = ftxui::Container::Horizontal({ left_menuz, right_menuz });
  renderer = Renderer(container, [&] {
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
}

void DBusScreenManager::SetupDBusConnection() {
  connection = std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system_mon());
  match = std::make_unique<sdbusplus::bus::match::match>(*connection, "",
                                                         [this](sdbusplus::message_t& msg) { this->match_callback(msg); });
  auto mc = connection->new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus.Monitoring",
                                        "BecomeMonitor");
  mc.append<std::vector<std::string>, uint32_t>({ "path=/com/skaginn3x/Signals" }, 0);
  auto reply = connection->call(mc);
  if (reply.is_method_error()) {
    if (reply.get_error() != nullptr)
      throw std::runtime_error(reply.get_error()->message);
    throw std::runtime_error("Unknown error");
  }
}

void DBusScreenManager::match_callback(sdbusplus::message_t& msg) {
  if (msg.get_member() && std::string(msg.get_member()) == "PropertiesChanged") {
    std::tuple<std::string, std::vector<std::pair<std::string, std::variant<bool, uint64_t>>>, std::vector<std::string>>
        container{};
    if (sdbusplus::utility::read_into_tuple(container, msg)) {
      std::string const& interface = std::get<0>(container);
      auto position = std::find(noticed_interfaces.begin(), noticed_interfaces.end(), interface);
      auto value = std::get<1>(container)[0].second;
      if (position == noticed_interfaces.end()) {
        noticed_interfaces.emplace_back(interface);
        values.emplace_back(value);
        right_menuz->Render();
      } else {
        auto index = position - noticed_interfaces.begin();
        if (values[index] != value) {
          if (index == left_menu_selected) {
            values[index] = value;

            std::string current_value = std::visit(VariantToString(), values[index]);
            entries = { current_value };
            right_menuz->Render();
            screen.Post(ftxui::Event::Custom);
          }
        }
      }
      // std::visit([&interface](auto& value) { std::cout << interface << ": " << value << std::endl; },
      // std::get<1>(container)[0].second);
    }
  }
}

auto DBusScreenManager::left_menu() -> ftxui::Component {
  auto option = ftxui::MenuOption::Vertical();

  option.on_enter = [this] {
    std::string current_value = std::visit(VariantToString(), values[left_menu_selected]);
    entries = { current_value };
    right_menuz->TakeFocus();
    screen.Post(ftxui::Event::Custom);
  };

  return ftxui::Menu(&noticed_interfaces, &left_menu_selected, option);
}

auto DBusScreenManager::right_menu() -> ftxui::Component {
  auto option = ftxui::MenuOption::Vertical();
  // option.on_enter = screen.ExitLoopClosure();
  return ftxui::Menu(&entries, &right_menu_selected, option);
}
