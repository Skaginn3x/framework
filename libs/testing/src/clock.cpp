#include <tfc/testing/clock.hpp>

namespace tfc::testing {

namespace {
thread_local clock::time_point ticks{};
}

auto clock::now() noexcept -> clock::time_point {
  return ticker();
}
void clock::set_ticks(clock::time_point nticks) {
  ticker() = nticks;
}
auto clock::ticker() noexcept -> clock::time_point& {
  return ticks;
}

}  // namespace tfc::testing
