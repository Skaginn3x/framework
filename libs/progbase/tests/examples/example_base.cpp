#include "tfc/progbase.hpp"
#include <boost/program_options.hpp>

int main (int argc, char** argv) {
  auto prog_desc{tfc::base::default_description};
  prog_desc.add_options()("custom_opts,c", boost::program_options::value<bool>(), "This is a help text for custom_opts");
  tfc::base::init(argc, argv, prog_desc);
}
