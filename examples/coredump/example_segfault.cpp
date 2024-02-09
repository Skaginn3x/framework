#include <tfc/progbase.hpp>

#include "segfault.hpp"

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  produce_segfault();

  throw std::runtime_error("This is a test");  // NOSONAR
}
