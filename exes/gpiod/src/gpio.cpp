#include "gpio.hpp"

namespace tfc {

gpio::gpio(asio::io_context& ctx, std::filesystem::path const& char_device)
    : ctx_{ ctx }, chip_{ char_device },
      config_{ ctx, "gpio-map", std::bind_front(&gpio::init, this), config_t::storage_t{ chip_.get_info().num_lines() } },
      pins_{ config_->size() }, logger_{ "gpio" } {
  for (std::size_t idx{ 0 }; auto const& pin_config : config_.get()) {
    pin_config.active.observe(std::bind_front(&gpio::pin_active_change, this, idx));
    pin_config.direction.observe(std::bind_front(&gpio::pin_direction_change, this, idx));
    idx++;
  }
}

void gpio::init(config_t const&) {
  logger_.info("GPIO started");
}
void gpio::pin_active_change(pin_index_t idx,
                             [[maybe_unused]] gpiod::line::value new_value,
                             [[maybe_unused]] gpiod::line::value old_value) {
  if (new_value == gpiod::line::value::INACTIVE) {
//    config_->at(idx).in_or_out = std::monostate{};
    pins_.at(idx) = std::monostate{};
  }
}
void gpio::pin_direction_change(pin_index_t,
                                [[maybe_unused]] gpiod::line::direction new_value,
                                [[maybe_unused]] gpiod::line::direction old_value) {}
void gpio::pin_edge_change(pin_index_t,
                           [[maybe_unused]] gpiod::line::edge new_value,
                           [[maybe_unused]] gpiod::line::edge old_value) {}
void gpio::pin_bias_change(pin_index_t,
                           [[maybe_unused]] gpiod::line::bias new_value,
                           [[maybe_unused]] gpiod::line::bias old_value) {}
void gpio::pin_force_change(pin_index_t,
                            [[maybe_unused]] pin::out::force_e new_value,
                            [[maybe_unused]] pin::out::force_e old_value) {}
void gpio::pin_drive_change(pin_index_t,
                            [[maybe_unused]] gpiod::line::drive new_value,
                            [[maybe_unused]] gpiod::line::drive old_value) {}
void gpio::pin_event(pin_index_t, bool) {}

}  // namespace tfc
