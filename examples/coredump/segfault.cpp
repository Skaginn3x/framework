#include "segfault.hpp"

void produce_segfault() {
  int* ptr = nullptr;
  *ptr = 42;  // NOSONAR
}
