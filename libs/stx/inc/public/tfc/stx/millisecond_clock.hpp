#pragma once

#include <chrono>

namespace tfc::stx {

struct millisecond_system_clock
{
  using duration = std::chrono::milliseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<millisecond_system_clock, duration>;

  static_assert(millisecond_system_clock::duration::min()
                    < millisecond_system_clock::duration::zero(),
                "a clock's minimum duration cannot be less than its epoch");

  static constexpr bool is_steady{ false };

  static time_point now() noexcept {
      return time_point{ std::chrono::duration_cast<duration>(std::chrono::system_clock::now().time_since_epoch()) };
  }

  // Map to C API
  static std::time_t to_time_t(time_point const& time) noexcept
  {
      using system_clock = std::chrono::system_clock;
      return system_clock::to_time_t( system_clock::time_point{ std::chrono::duration_cast<system_clock::duration>(time.time_since_epoch()) } );
  }

  static time_point
  from_time_t(std::time_t time) noexcept
  {
      using system_clock = std::chrono::system_clock;
      auto system_time_point{ system_clock::from_time_t(time) };
      return time_point{ std::chrono::duration_cast<duration>(system_time_point.time_since_epoch()) };
  }
};


}