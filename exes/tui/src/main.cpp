// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <cstdio>
#include <fstream>
#include <iostream>

#include <tfc/progbase.hpp>

#include <boost/program_options.hpp>
#include <glaze/glaze.hpp>

#include "keybinding.hpp"
#include "main_ui.hpp"
#include "version.hpp"

using JSON = glz::json_t;
bool ParseJSON(std::string input, JSON& out);

int main(int argument_count, const char** arguments) {
  auto desc{ tfc::base::default_description() };
//  desc.add_options()("") // todo
  tfc::base::init(argument_count, arguments, desc);
//  args::ArgumentParser args("");
//  args.Prog("json-tui");
//  args.Description("A JSON terminal UI");
//  args.Epilog(
//      "If no file is given, json-tui reads JSON from the standard input\n"
//      "\n"
//      "Please report bugs to:"
//      "https://github.com/ArthurSonzogni/json-tui/issues");
//
//  args::Positional<std::string> file(args, "file",
//                                     "A JSON file. Omit to read from stdin.");
//  args::Flag help(args, "help", "Display this help menu.", {'h', "help"});
//  args::Flag version(args, "version", "Print version.", {'v', "version"});
//  args::Flag keybinding(args, "keybinding", "Display key binding.",
//                        {'k', "key", "keybinding"});
//  args::Flag fullscreen(
//      args, "fullscreen",
//      "Display the JSON in fullscreen, in an alternate buffer",
//      {'f', "fullscreen"});
//  bool success = args.ParseCLI(argument_count, arguments);
//  if (!success) {
//    std::cerr << "Invalid arguments" << std::endl;
//    return EXIT_FAILURE;
//  }

//  if (help) {
//    std::cout << args;
//    return EXIT_SUCCESS;
//  }

//  if (version) {
//    std::cout << project_version << std::endl;
//    return EXIT_SUCCESS;
//  }

//  if (keybinding) {
//    KeyBinding();
//    return EXIT_SUCCESS;
//  }

  std::stringstream ss;
//  if (file) {
//    auto file_stream = std::ifstream(args::get(file));
//    if (!file_stream) {
//      std::cerr << "Could not open file " << args::get(file) << std::endl;
//      return EXIT_FAILURE;
//    }
//    ss << file_stream.rdbuf();
//  } else {
//#if defined(_WIN32)
//    std::cerr << "Error. Reading from stdin is not supported on Windows. "
//                 "Please provide a file."
//              << std::endl;
//
//    std::cout << args;
//    return EXIT_FAILURE;
//#else
//    std::cout << "Reading from stdin..." << std::flush;
//    ss << std::cin.rdbuf();
//    stdin = freopen("/dev/tty", "r", stdin);
//#endif
//  }

  JSON json;
//  if (!ParseJSON(ss.str(), json))
//    return EXIT_FAILURE;

  DisplayMainUI(json, true); // fullscreen
  return EXIT_SUCCESS;
}

//bool ParseJSON(std::string input, JSON& out) {
//  class JsonParser : public nlohmann::detail::json_sax_dom_parser<JSON> {
//   public:
//    JsonParser(JSON& j)
//        : nlohmann::detail::json_sax_dom_parser<JSON>(j, false) {}
//    bool parse_error(std::size_t /*position*/,
//                     const std::string& /*last_token*/,
//                     const JSON::exception& ex) {
//      std::cerr << std::endl;
//      std::cerr << ex.what() << std::endl;
//      return false;
//    }
//  };
//  JsonParser parser(out);
//  return JSON::sax_parse(input, &parser);
//}
