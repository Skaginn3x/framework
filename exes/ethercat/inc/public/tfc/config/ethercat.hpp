#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

// namespace tfc::ethercat::config {
//     enum struct port_e : uint16_t {
//         mqtt = 1883, mqtts = 8883
//     };
// }
//
// template<>
// struct glz::meta<tfc::ethercat::config::port_e> {
//     using
//     enum tfc::ethercat::config::port_e;
//     static constexpr std::string_view name{"tfc::mqtt::port_e"};
//     static constexpr auto value = glz::enumerate("mqtt", mqtt, "mqtts", mqtts);
// };

namespace tfc::ethercat::config {

namespace asio = boost::asio;

struct ethercat {
  std::string interface {};

  struct glaze {
    // clang-format off
            static constexpr auto value{glz::object(
                    "interface", &ethercat::interface
            )};
    // clang-format on
    static constexpr std::string_view name{ "ethercat" };
  };
};
}  // namespace tfc::ethercat::config
