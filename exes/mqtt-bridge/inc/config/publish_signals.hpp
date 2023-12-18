#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/ipc/enums.hpp>
#include <tfc/ipc/glaze_meta.hpp>

namespace tfc::mqtt::config {

namespace asio = boost::asio;

struct publish_signals {
  std::vector<std::string> publish_signals{};

  struct glaze {
    // clang-format off
            static constexpr auto value{glz::object(
                    "publish_signals", &publish_signals::publish_signals, "Signals to publish"
            )};
    // clang-format on
    static constexpr std::string_view name{ "publish_signals" };
  };
};

}  // namespace tfc::mqtt::config
