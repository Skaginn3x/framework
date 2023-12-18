#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace tfc::mqtt::config {

namespace asio = boost::asio;

struct publish_signals_owner_mock {
  std::vector<std::string> publish_signals{};
};

class publish_signals_mock {
  publish_signals_owner_mock owner_;

public:
  publish_signals_mock(asio::io_context const&, std::string const&) {
    owner_.publish_signals = { "test_mqtt_bridge.def.bool.bool_signal", "second", "third" };
  }

  [[nodiscard]] auto value() const -> publish_signals_owner_mock const& { return owner_; }
};

}  // namespace tfc::mqtt::config
