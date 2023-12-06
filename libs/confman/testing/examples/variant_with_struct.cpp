#include <string_view>
#include <variant>
#include <iostream>

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
  // std::variant<port_e, uint16_t> port{};
  port_e port{};

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

  switch (config.value().port) {
    case port_e::mqtt:
      std::cout << "mqtt" << std::endl;
      break;
    case port_e::mqtts:
      std::cout << "mqtts" << std::endl;
      break;
  }

  ctx.run();
  return 0;
}
