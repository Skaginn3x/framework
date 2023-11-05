import tfc.base;

import argparse;

auto main(int argc, char** argv) -> int {
  auto prog{ tfc::base::default_parser() };
  prog.add_argument("-c", "--custom")
      .default_value(false)
      .help("This is a help text for custom_opts");
  tfc::base::init(argc, argv, prog);
}
