#include "gpio.hpp"

namespace tfc {

gpio::gpio(asio::io_context& ctx, std::filesystem::path const& char_device)
    : ctx_{ ctx }, chip_{ char_device }, config_{ ctx, "lines", std::vector<tfc::confman::observable<pin_t>>{ chip_.get_info().num_lines() } },
      manager_client_(ctx_), pins_{ config_->size() }, logger_{ "gpio" }, chip_asio_{ ctx, chip_.fd() } {
  for (std::size_t idx{ 0 }; [[maybe_unused]] auto const& pin_config : config_.value()) {
    pin_config.observe(std::bind_front(&gpio::pin_direction_change, this, idx));
    pin_direction_change(idx, pin_config.value(), pin_config.value());
    idx++;
  }
  chip_asio_.async_wait(boost::asio::posix::descriptor_base::wait_read, std::bind_front(&gpio::chip_ready_to_read, this));
}

void gpio::pin_direction_change(pin_index_t idx,
                                pin_t const& new_value,
                                [[maybe_unused]] pin_t const& old_value) noexcept {
  try {
    logger_.trace(R"(Got new direction change with new value: "{}", old value: "{}")", glz::write_json(new_value),
                  glz::write_json(old_value));
    std::visit(
        [this, idx](auto& val) {
          using visit_t = std::remove_cvref_t<decltype(val)>;
          if constexpr (std::is_same_v<visit_t, pin::in>) {
            pins_.at(idx).emplace<ipc_output_t>(ctx_, manager_client_, fmt::format("out.{}", idx));
            val.edge.observe(std::bind_front(&gpio::pin_edge_change, this, idx));
            val.bias.observe(std::bind_front(&gpio::pin_bias_change, this, idx));
          } else if constexpr (std::is_same_v<visit_t, pin::out>) {
            val.force.observe(std::bind_front(&gpio::pin_force_change, this, idx));
            val.drive.observe(std::bind_front(&gpio::pin_drive_change, this, idx));
          } else {
          }
        },
        new_value);
  } catch (std::exception const& exception) {  // todo make this general for all observable callbacks
    fmt::print(stderr, "Got exception: \"{}\"", exception.what());
    tfc::base::terminate();
  }
}
void gpio::pin_edge_change(pin_index_t idx,
                           gpiod::line::edge new_value,
                           [[maybe_unused]] gpiod::line::edge old_value) noexcept {
  logger_.trace(R"(Got new edge change with new value: "{}", old value: "{}")", glz::write_json(new_value),
                glz::write_json(old_value));
  auto settings{ chip_.prepare_request().get_line_config().get_line_settings().at(idx) };
  settings.set_edge_detection(new_value);
  chip_.prepare_request().add_line_settings(idx, settings);
  if (new_value != gpiod::line::edge::NONE) {
    chip_.watch_line_info(idx);
  } else {
    chip_.unwatch_line_info(idx);
  }
}
void gpio::pin_bias_change(pin_index_t idx,
                           gpiod::line::bias new_value,
                           [[maybe_unused]] gpiod::line::bias old_value) noexcept {
  logger_.trace(R"(Got new bias change with new value: "{}", old value: "{}")", glz::write_json(new_value),
                glz::write_json(old_value));
  auto settings{ chip_.prepare_request().get_line_config().get_line_settings().at(idx) };
  settings.set_bias(new_value);
  chip_.prepare_request().add_line_settings(idx, settings).do_request();
}
void gpio::pin_force_change([[maybe_unused]] pin_index_t idx,
                            pin::out::force_e new_value,
                            [[maybe_unused]] pin::out::force_e old_value) noexcept {
  logger_.trace(R"(Got new force change with new value: "{}", old value: "{}")", glz::write_json(new_value),
                glz::write_json(old_value));
//  auto settings{ chip_.prepare_request().get_line_config().get_line_settings().at(idx) };
//  switch (new_value) {
//    using enum pin::out::force_e;
//    case as_is:
//      break;
//    case on: {
//      auto change_pin{ config_.make_change()->at(idx) };
//      if (auto* out{ std::get_if<pin::out>(&change_pin.in_or_out) }) {
//        out->force.set(as_is);
//      }
//      settings.set_output_value(gpiod::line::value::ACTIVE);
//      break;
//    }
//    case save_on:
//      settings.set_output_value(gpiod::line::value::ACTIVE);
//      break;
//    case off: {
//      auto change_pin{ config_.make_change()->at(idx) };
//      if (auto* out{ std::get_if<pin::out>(&change_pin.in_or_out) }) {
//        out->force.set(as_is);
//      }
//      settings.set_output_value(gpiod::line::value::INACTIVE);
//      break;
//    }
//    case save_off:
//      settings.set_output_value(gpiod::line::value::INACTIVE);
//      break;
//  }
//  chip_.prepare_request().add_line_settings(idx, settings).do_request();
}
void gpio::pin_drive_change(pin_index_t idx,
                            gpiod::line::drive new_value,
                            [[maybe_unused]] gpiod::line::drive old_value) noexcept {
  logger_.trace(R"(Got new drive change with new value: "{}", old value: "{}")", glz::write_json(new_value),
                glz::write_json(old_value));
  auto settings{ chip_.prepare_request().get_line_config().get_line_settings().at(idx) };
  settings.set_drive(new_value);
  chip_.prepare_request().add_line_settings(idx, settings).do_request();
}
void gpio::pin_event(pin_index_t idx, bool state) noexcept {
  if (auto* output{ std::get_if<ipc_output_t>(&pins_.at(idx)) }) {
    logger_.trace(R"(Got event on pin: "{}" with state: "{}" but could not send it)", idx, state);
    output->send(state);
  } else {
    logger_.warn(R"(Got event on pin: "{}" with state: "{}" but could not send it)", idx, state);
  }
}
void gpio::ipc_event(gpio::pin_index_t idx, bool state) noexcept {
  logger_.trace(R"(Got event on ipc: "{}" with state: "{}" but could not send it)", idx, state);
  auto settings{ chip_.prepare_request().get_line_config().get_line_settings().at(idx) };
  settings.set_output_value(static_cast<gpiod::line::value>(state));
  chip_.prepare_request().add_line_settings(idx, settings).do_request();
}
void gpio::chip_ready_to_read(std::error_code const& err) noexcept {
  if (err) {
    logger_.warn("Unexpected error for gpio chip file descriptor, error: \"{}\"", err.message());
    return;
  }
  // todo log timestamp from event vs current time
  try {
    [[maybe_unused]] auto event{ chip_.read_info_event() };
    auto offset = chip_.read_info_event().get_line_info().offset();
    auto settings{ chip_.prepare_request().get_line_config().get_line_settings().at(offset) };
    pin_event(offset, static_cast<bool>(settings.output_value()));
  } catch (std::exception const& exc) {
    logger_.info("Chip event got an exception: {}", exc.what());
  }
  chip_asio_.async_wait(boost::asio::posix::descriptor_base::wait_read, std::bind_front(&gpio::chip_ready_to_read, this));
}

}  // namespace tfc
