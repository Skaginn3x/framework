#include "tfc/progbase.hpp"
#include "tfc/logger.hpp"
#include <boost/program_options.hpp>

int main(int argc, char** argv){
  // base::init reads the log lvl, program id and stdout parameters from command line.
  auto prog_desc{tfc::base::default_description()};
  tfc::base::init(argc, argv, prog_desc);

  // Create a instance of a logger
  tfc::logger::logger logger_instance("tank1");
  logger_instance.log<tfc::logger::lvl_e::critical>("Very bad message");

  // If you want the logger to also log to the console pass --stdout true to the command line
  logger_instance.log<tfc::logger::lvl_e::info>("Just informative");
}
