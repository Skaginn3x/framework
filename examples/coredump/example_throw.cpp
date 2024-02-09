#include <tfc/progbase.hpp>

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  throw std::runtime_error("This is a test");
}
