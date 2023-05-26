#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;
namespace ipc = tfc::ipc;


#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Button, Horizontal, Renderer
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for separator, gauge, text, Element, operator|, vbox, border
#include <ftxui/component/loop.hpp>

// https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/button.cpp

using namespace ftxui;

struct app {
  static std::chrono::milliseconds constexpr poll_rate{ 100 };
  app(asio::io_context& ctx) : buttons_{ Container::Horizontal({
            Button("Decrease", [&] { value--; }),
            Button("Increase", [&] { value++; }),
        }) }, component_{ Renderer(buttons_, [this] {
          return vbox({
                     text("value = " + std::to_string(value)),
                     separator(),
                     gauge(value * 0.01f),
                     separator(),
                     buttons_->Render(),
                 }) |
                 border;
        }) },
        screen_{ ScreenInteractive::FitComponent() }, loop_{ &screen_, component_ }, ctx_{ ctx } {
    timer_.expires_after(poll_rate);
    timer_.async_wait(std::bind_front(&app::poll_ftxui, this));

    ipc_client_.signals(std::bind_front(&app::signals_cb, this));
    ipc_client_.slots(std::bind_front(&app::slots_cb, this));
    ipc_client_.connections(std::bind_front(&app::connections_cb, this));
  }

  void poll_ftxui(std::error_code const& err) {
    if(err) {
      return;
    }
    if (loop_.HasQuitted()) {
      ctx_.stop();
    }
    else {
      loop_.RunOnce();
      timer_.expires_after(poll_rate);
      timer_.async_wait(std::bind_front(&app::poll_ftxui, this));
    }
  }

  void signals_cb(std::vector<tfc::ipc_ruler::signal> const& signals) {
    signals_ = signals;
  }

  void slots_cb(std::vector<tfc::ipc_ruler::slot> const& slots) {
    slots_ = slots;
  }

  void connections_cb(std::map<std::string, std::vector<std::string>> connections) {
    connections_ = connections;
  }

  int value = 50;

  Component buttons_;
  Component component_;
  ScreenInteractive screen_;
  Loop loop_;

  asio::io_context& ctx_;
  asio::steady_timer timer_{ ctx_ };
  tfc::ipc_ruler::ipc_manager_client ipc_client_{ ctx_ };
  std::vector<tfc::ipc_ruler::signal> signals_{};
  std::vector<tfc::ipc_ruler::slot> slots_{};
  std::map<std::string, std::vector<std::string>> connections_{};
};

int main(int argc, const char* argv[]) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  app const my_app{ ctx };

  ctx.run();

  return EXIT_SUCCESS;
}
