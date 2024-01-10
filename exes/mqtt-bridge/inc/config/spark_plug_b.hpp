#pragma once

#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/utils/json_schema.hpp>

namespace tfc::mqtt::config {

namespace asio = boost::asio;

struct spark_plug_b {
  std::string node_id{ "tfc_unconfigured_node_id" };
  std::string group_id{ "tfc_unconfigured_group_id" };

  struct glaze {
    // clang-format off
            static constexpr auto value{glz::object(
                    "node_id", &spark_plug_b::node_id,
                    tfc::json::schema{.description = "Spark Plug B Node ID, used to identify which node is sending information", .pattern = "[^+#/]"},
                    "group_id", &spark_plug_b::group_id,
                    tfc::json::schema{.description = "Spark Plug B Group ID, used to identify which group the node belongs to", .pattern = "[^+#/]"}
            )};
    // clang-format on
    static constexpr std::string_view name{ "spark_plug_b" };
  };
};

}  // namespace tfc::mqtt::config
