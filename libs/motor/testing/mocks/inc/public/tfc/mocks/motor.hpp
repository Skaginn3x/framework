#pragma once

#include <tfc/motor.hpp>

namespace tfc::motor {

class mock_api {
  using config_t = typename tfc::motor::config_t;
  mock_api(std::shared_ptr<sdbusplus::asio::connection>, std::string_view, config_t = {}) {}




};

}  // namespace tfc::motor
