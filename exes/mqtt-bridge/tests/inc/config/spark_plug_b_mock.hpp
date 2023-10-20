#pragma once

#include <string>

#include <boost/asio.hpp>

namespace tfc::mqtt::config {

namespace asio = boost::asio;

struct spark_plug_b_owner_mock {
  std::string node_id{};
  std::string group_id{};
};

class spark_plug_b_mock {
  spark_plug_b_owner_mock owner_;

public:
  spark_plug_b_mock(asio::io_context const&, std::string const&) {
    owner_.group_id = "group_id";
    owner_.node_id = "node_id";
  }

  [[nodiscard]] auto value() const -> spark_plug_b_owner_mock { return owner_; }
};

}  // namespace tfc::mqtt::config
