#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

#include <tfc/logger.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>

namespace asio = boost::asio;

enum struct gpio_pin_behaviour_e : std::uint8_t {
  unknown = 0,
  output,
  input,
  input_pull_up,
  input_pull_down,
  input_interrupt,
  pwm,
  uart,
  spi,
  i2c,
  can,
  one_wire,
  clock,
  pcm
};

template <>
struct glz::meta<gpio_pin_behaviour_e> {
  using enum gpio_pin_behaviour_e;
  // clang-format off
  static constexpr auto value = enumerate("unknown", unknown,
                                          "output", output,
                                          "input", input,
                                          "input_pull_up", input_pull_up,
                                          "input_pull_down", input_pull_down,
                                          "input_interrupt", input_interrupt,
                                          "pwm", pwm,
                                          "uart", uart,
                                          "spi", spi,
                                          "i2c", i2c,
                                          "can", can,
                                          "one_wire", one_wire,
                                          "clock", clock,
                                          "pcm", pcm
  );
  // clang-format on
};

struct storage {
  tfc::confman::observable<std::unordered_map<std::string, gpio_pin_behaviour_e>> pins{};
};

template <>
struct glz::meta<storage> {
  static constexpr auto value{ glz::object("pins", &storage::pins) };
};

namespace tfc {

class gpio {
public:
  gpio(asio::io_context& ctx, std::filesystem::path char_device);

  void init();
private:
  asio::io_context& ctx_;
  std::filesystem::path char_device_{};
  tfc::confman::config<storage> config_;
  tfc::logger::logger logger_;
};

}  // namespace tfc