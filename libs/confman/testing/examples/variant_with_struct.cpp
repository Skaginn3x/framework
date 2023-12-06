#include <iostream>
#include <string_view>
#include <variant>

#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/confman.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;

enum struct port_e : uint16_t { mqtt = 1883, mqtts = 8883 };

template <>
struct glz::meta<port_e> {
  static constexpr std::string_view name{ "port_e" };
  static constexpr auto value = glz::enumerate("mqtt", port_e::mqtt, "mqtts", port_e::mqtts);
};

struct broker {
  std::variant<port_e, uint16_t> port{};
  // std::variant<std::string, uint16_t> port{};
  // port_e port{};

  struct glaze {
    static constexpr auto value{ glz::object("port", &broker::port) };
    static constexpr std::string_view name{ "broker" };
  };
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::confman::config<broker> config{ ctx, "key" };

  ctx.run_for(std::chrono::seconds{ 1 });

  // std::visit the conf variable and print out the result
  std::visit(
      [](auto&& arg) {
        if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) {
          std::cout << "string" << std::endl;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, uint16_t>) {
          std::cout << "uint16_t" << std::endl;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, port_e>) {
          std::cout << "port_e" << std::endl;
        } else {
          std::cout << "unknown" << std::endl;
        }
      },
      config.value().port);

  ctx.run();
  return 0;
}
