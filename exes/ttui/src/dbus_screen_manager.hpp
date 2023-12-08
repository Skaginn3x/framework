// DBusScreenManager.hpp

#pragma once

#include "ftxui/component/captured_mouse.hpp" // for ftxui
#include "ftxui/component/component.hpp"      // for Menu, Renderer, ScreenInteractive
#include "ftxui/component/component_options.hpp" // for MenuOption
#include "ftxui/component/loop.hpp"           // for Loop
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus/match.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <variant>
#include <string>

namespace asio = boost::asio;
namespace po = boost::program_options;


class DBusScreenManager {
public:
  DBusScreenManager();
  void Run();

private:
  ftxui::ScreenInteractive screen;
  ftxui::Component left_menuz, right_menuz, container;
  ftxui::Component renderer;
  std::vector<std::string> noticed_interfaces;
  std::vector<std::variant<bool, uint64_t>> values;
  std::vector<std::string> entries;
  int left_menu_selected = 0;
  int right_menu_selected = 0;
  asio::io_context ctx{};
  std::shared_ptr<sdbusplus::asio::connection> connection;
  std::__detail::__unique_ptr_t<sdbusplus::bus::match::match> match;

  void SetupComponents();
  void SetupDBusConnection();
  void match_callback(sdbusplus::message_t& msg);
  ftxui::Component left_menu();
  ftxui::Component right_menu();
  struct VariantToString {
    std::string operator()(bool v) const { return v ? "true" : "false"; }
    std::string operator()(int64_t v) const { return std::to_string(v); }
    std::string operator()(uint64_t v) const { return std::to_string(v); }
    std::string operator()(double v) const { return std::to_string(v); }
    std::string operator()(const std::string& v) const { return v; }
    std::string operator()(const std::vector<std::string>& v) const {
      std::string result;
      for (const auto& elem : v) {
        if (!result.empty()) {
          result += ", "; // Delimiter between elements
        }
        result += elem;
      }
      return result;
    }
  };
};
