#include <tfc/testing/asio_clock.hpp>

namespace tfc::testing {

namespace {
thread_local std::chrono::time_point<clock> ticks{};
}

auto clock::now() noexcept -> clock::time_point {
  return ticks;
}
void clock::set_ticks(clock::time_point nticks) {
  ticks = nticks;
}

}  // namespace tfc::testing
