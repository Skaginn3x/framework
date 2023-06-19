#pragma once
#include <string_view>

#include <tfc/cia/402.hpp>

namespace tfc::ec::devices::schneider::lxm32m::pdos {
using std::string_view_literals::operator""sv;

struct rx_cyclic_synchronous_position {
  static constexpr auto name{ "rx_cyclic_synchronous_position" };
};
struct tx_cyclic_synchronous_position {
  static constexpr auto name{ "tx_cyclic_synchronous_position" };
};

struct statistics {};

}  // namespace tfc::ec::devices::schneider::lxm32m::pdos
