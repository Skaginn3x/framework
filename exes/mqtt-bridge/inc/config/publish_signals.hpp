#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <glaze/core/common.hpp>

namespace tfc::mqtt::config {

struct publish_signals {
  std::vector<std::string> publish_signals{};

  struct glaze {
    static constexpr auto value{ glz::object("publish_signals", &publish_signals::publish_signals, "Signals to publish") };
    static constexpr std::string_view name{ "publish_signals" };
  };
};

}  // namespace tfc::mqtt::config
