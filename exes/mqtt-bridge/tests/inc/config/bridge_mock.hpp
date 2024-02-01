#pragma once

#include <boost/asio.hpp>

#include "../inc/structs.hpp"
#include "../inc/config/bridge.hpp"

namespace asio = boost::asio;

namespace tfc::mqtt::config {

struct bridge_owner_mock {
  std::string address{};
  structs::ssl_active_e ssl_active{};
  std::string username{};
  std::string password{};
  std::string client_id{};
  std::vector<signal_name> publish_signals{};
  std::string node_id{};
  std::string group_id{};
  std::vector<signal_definition> writeable_signals{};

  static auto get_port() -> std::string { return "1883"; }
};

class bridge_mock {
  bridge_owner_mock owner_;

public:
  bridge_mock(asio::io_context const&, std::string const&) {
    owner_.address = "localhost";
    owner_.ssl_active = structs::ssl_active_e::no;
    owner_.username = "";
    owner_.password = "";
    owner_.client_id = "cid1";
    owner_.publish_signals = { signal_name{ "integration_tests.def.bool.test" } };
    owner_.node_id = "tfc_unconfigured_node_id";
    owner_.group_id = "tfc_unconfigured_group_id";
    owner_.writeable_signals = {};
  }

  [[nodiscard]] auto value() const -> bridge_owner_mock { return owner_; }
};

}  // namespace tfc::mqtt::config
