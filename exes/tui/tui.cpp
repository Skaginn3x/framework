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

// https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/button.cpp

using namespace ftxui;

int main(int argc, const char* argv[]) {
  tfc::base::init(argc, argv);

  int value = 50;

  // The tree of components. This defines how to navigate using the keyboard.
  auto buttons = Container::Horizontal({
      Button("Decrease", [&] { value--; }),
      Button("Increase", [&] { value++; }),
  });

  // Modify the way to render them on screen:
  auto component = Renderer(buttons, [&] {
    return vbox({
               text("value = " + std::to_string(value)),
               separator(),
               gauge(value * 0.01f),
               separator(),
               buttons->Render(),
           }) |
           border;
  });

  auto screen = ScreenInteractive::FitComponent();
  screen.Loop(component);
  return 0;
}

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

