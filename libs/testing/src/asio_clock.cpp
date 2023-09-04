#include <tfc/testing/asio_clock.hpp>

namespace tfc::testing {

auto clock::now() noexcept -> clock::time_point {
  return ticker();
}
void clock::set_ticks(clock::time_point nticks) {
  ticker() = nticks;
}
auto clock::ticker() noexcept -> clock::time_point& {
  static clock::time_point ticks{};
  return ticks;
}

}  // namespace tfc::testing
