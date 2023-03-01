#include <string>
#include <array>
#include <glaze/glaze.hpp>
#include <boost/program_options.hpp>
#include <magic_enum.hpp>

#include "tfc/progbase.hpp"
#include "tfc/logger.hpp"

int main(int argc, char** argv) {
  auto prog_desc{tfc::base::default_description()};
  tfc::base::init(argc, argv, prog_desc);

  using logger = tfc::logger::logger;
  using lvl = tfc::logger::lvl_e;

  logger a("Schema testing");

  std::string buffer = R"({"i":287,"d":3.14,"hello":"Hello World","arr":[1,2,3]})";
  auto s = glz::read_json<glz::json_t>(buffer);

  auto& d = s["d"].get<double>();
  a.log<lvl::info> ("{}", d);
  s["d"] = "asdf";


  std::string buffer2;
  glz::write<glz::opts{.prettify = true}>(s, buffer2);
  a.log<lvl::info>("\n{}", buffer2);
}
