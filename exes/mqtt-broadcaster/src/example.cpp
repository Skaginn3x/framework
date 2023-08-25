#include "config.hpp"
#include <iostream>

auto main() -> int {

  config cfg{};

  cfg.signals.push_back(Signal{ "signal1", 1 });

  for (auto& sig: cfg.signals) {
    std::cout << sig.name << " " << sig.value << std::endl;
  }

  return 0;
}