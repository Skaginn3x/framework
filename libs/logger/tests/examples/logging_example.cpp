#include <boost/program_options.hpp>
#include <string_view>
#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"

using std::string_view_literals::operator""sv;

auto main(int argc, char** argv) -> int {
  // base::init reads the log lvl, program id and stdout parameters from command line.
  tfc::base::init(argc, argv);

  // Create a instance of a logger
  tfc::logger::logger logger_instance("tank1");
  logger_instance.log<tfc::logger::lvl_e::critical>("Very bad message"sv);

  // If you want the logger to also log to the console pass --stdout true to the command line
  logger_instance.log<tfc::logger::lvl_e::info>("Just informative"sv);
}
