#pragma once

#include <chrono>

namespace tfc::testing {

// a bit delicate working with more than one timer instance at once
struct clock {
  using duration = std::chrono::nanoseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<clock>;
  static auto now() noexcept -> time_point;
  static void set_ticks(time_point nticks);
  static constexpr bool is_steady = true;

private:
  static auto ticker() noexcept -> time_point&;
};

struct wait_traits {
  static auto to_wait_duration(const clock::duration&) -> clock::duration { return std::chrono::nanoseconds(0); }
};

}  // namespace tfc::testing
