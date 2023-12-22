#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace tfc::mqtt::config {

namespace asio = boost::asio;

struct sig_name {
  std::string value{};
};

struct publish_signals_owner_mock {
  std::vector<sig_name> publish_signals{};
};

class publish_signals_mock {
  publish_signals_owner_mock owner_;

public:
  publish_signals_mock(asio::io_context const&, std::string const&) {
    owner_.publish_signals = { sig_name{ "test_mqtt_bridge.def.bool.bool_signal" }, sig_name{ "second" },
                               sig_name{ "third" } };
  }

  [[nodiscard]] auto value() const -> publish_signals_owner_mock const& { return owner_; }
};

}  // namespace tfc::mqtt::config
