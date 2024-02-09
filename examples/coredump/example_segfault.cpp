#include <tfc/progbase.hpp>

#include "segfault.hpp"

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  produce_segfault();
}
