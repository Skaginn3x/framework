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
using tfc::ipc::details::type_e;

struct signal_definition {
  std::string name{};
  std::string description{};
  type_e type{ type_e::unknown };
  struct glaze {
    // clang-format off
            static constexpr auto value{glz::object(
                    "name", &signal_definition::name,
                    "description", &signal_definition::description,
                    "type", &signal_definition::type
            )};
    // clang-format on
    static constexpr std::string_view name{ "tfc::mqtt::signal_definition" };
  };
};

struct writeable_signals {
  std::vector<signal_definition> writeable_signals{};

  struct glaze {
    // clang-format off
            static constexpr auto value{glz::object(
                    "writeable_signals", &writeable_signals::writeable_signals,
                    "Array of signals that an MQTT client with a Spark Plug B extension can write to"
            )};
    // clang-format on
    static constexpr std::string_view name{ "writeable_signals" };
  };
};

}  // namespace tfc::mqtt::config
