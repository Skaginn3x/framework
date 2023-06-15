#include <fmt/core.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <tfc/progbase.hpp>
#include <tfc/utils/pragmas.hpp>

namespace py = pybind11;
namespace asio = boost::asio;

PRAGMA_CLANG_WARNING_PUSH_OFF(-Wunsafe-buffer-usage) // todo fix
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wmissing-variable-declarations) // todo fix
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors) // todo fix
PYBIND11_EMBEDDED_MODULE(asio, mod) {
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
  mod.doc() = "Boost asio module";
  using ctx = asio::io_context;
  py::class_<ctx>(mod, "io_context")
      .def(py::init())
      .def(py::init<int>())
      .def_property_readonly("stopped", &ctx::stopped)
      .def("stop", &ctx::stop)
      .def("run", [](ctx& self){ return self.run(); }); // why?
}

PRAGMA_CLANG_WARNING_PUSH_OFF(-Wunsafe-buffer-usage) // todo fix
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wmissing-variable-declarations) // todo fix
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors) // todo fix
PYBIND11_EMBEDDED_MODULE(tfc, mod) {
  PRAGMA_CLANG_WARNING_POP
  PRAGMA_CLANG_WARNING_POP
  PRAGMA_CLANG_WARNING_POP
  mod.doc() = "Time for change module";
  mod.def("exit_signals", [](asio::io_context& ctx){
    asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);
  });
}

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };
  std::string filename{};
  // todo unnamed argument
  desc.add_options()("file,f", boost::program_options::value<std::string>(&filename), "Filename");
  // Todo add some known python arguments

  tfc::base::init(argc, argv, desc);

  pybind11::scoped_interpreter const guard{};

  py::eval_file(filename);

  // example of file
/*
from asio import io_context
import tfc

ctx = io_context()

tfc.exit_signals(ctx)

ctx.run()

print("hello world")
*/
}
