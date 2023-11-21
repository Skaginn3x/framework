#pragma once

#include <functional>

#include <boost/asio/io_context.hpp>

namespace tfc::sensor::control {
namespace asio = boost::asio;

class api {
public:
  api(std::function<void()> stop_motor, std::function<void()> start_motor)
    : stop_motor_{ stop_motor }, start_motor_{ start_motor } {
  }

  auto stop_motor() const noexcept -> void {
    stop_motor_();
  }

  auto start_motor() const noexcept -> void {
    start_motor_();
  }

private:
  std::function<void()> stop_motor_{};
  std::function<void()> start_motor_{};
};
}
