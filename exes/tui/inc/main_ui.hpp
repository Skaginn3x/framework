// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef JSON_TUI_MAIN_UI_HPP
#define JSON_TUI_MAIN_UI_HPP

#include <glaze/glaze.hpp>
#include <boost/asio/io_context.hpp>

void DisplayMainUI(boost::asio::io_context&, const glz::json_t& json, bool fullscreen);

#endif /* json_tui_main_ui_hpp */
