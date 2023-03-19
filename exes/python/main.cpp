
#include <tfc/progbase.hpp>
#include <boost/program_options.hpp>
#include <pybind11/embed.h>


auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };
  desc.add_options()
      ("")

  tfc::base::init(argc, argv, );

  pybind11::scoped_interpreter const guard{};

  pybind11::module foo = pybind11::module::import("tfclogger");

}
