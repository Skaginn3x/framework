#include "gpio.h"

#include <gpiod.hpp>

namespace tfc {

// clang-format off
static const storage raspberry_pi_4{ .pins = tfc::confman::observable<std::unordered_map<std::string, gpio_pin_behaviour_e>>{ {
                                         { "GPIO0", gpio_pin_behaviour_e::input },
                                         { "GPIO1", gpio_pin_behaviour_e::input },
                                         { "GPIO2", gpio_pin_behaviour_e::i2c },
                                         { "GPIO3", gpio_pin_behaviour_e::i2c },
                                         { "GPIO4", gpio_pin_behaviour_e::input },
                                         { "GPIO5", gpio_pin_behaviour_e::input },
                                         { "GPIO6", gpio_pin_behaviour_e::input },
                                         { "GPIO7", gpio_pin_behaviour_e::input },
                                         { "GPIO8", gpio_pin_behaviour_e::input },
                                         { "GPIO9", gpio_pin_behaviour_e::pwm },
                                         { "GPIO10", gpio_pin_behaviour_e::spi },
                                         { "GPIO11", gpio_pin_behaviour_e::spi },
                                         { "GPIO12", gpio_pin_behaviour_e::pwm },
                                         { "GPIO13", gpio_pin_behaviour_e::spi },
                                         { "GPIO14", gpio_pin_behaviour_e::clock },
                                         { "GPIO15", gpio_pin_behaviour_e::clock },
                                         { "GPIO16", gpio_pin_behaviour_e::input },
                                         { "GPIO17", gpio_pin_behaviour_e::input },
                                         { "GPIO18", gpio_pin_behaviour_e::pcm },
                                         { "GPIO19", gpio_pin_behaviour_e::pcm },
                                         { "GPIO20", gpio_pin_behaviour_e::output },
                                         { "GPIO21", gpio_pin_behaviour_e::output },
                                         { "GPIO22", gpio_pin_behaviour_e::output },
                                         { "GPIO23", gpio_pin_behaviour_e::output },
                                         { "GPIO24", gpio_pin_behaviour_e::output },
                                         { "GPIO25", gpio_pin_behaviour_e::output },
                                         { "GPIO26", gpio_pin_behaviour_e::output },
                                         { "GPIO27", gpio_pin_behaviour_e::output }
                                     } } };
// clang-format on

gpio::gpio(asio::io_context& ctx, std::filesystem::path char_device)
    : ctx_{ ctx }, char_device_{ std::move(char_device) },
      config_{ ctx, "gpio-map", [this]([[maybe_unused]] auto const& self) { this->init(); }, raspberry_pi_4 }, logger_{"gpio"} {}

void gpio::init() {
  logger_.info("GPIO started");
}

}  // namespace tfc
