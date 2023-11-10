#pragma once

#include "sensor_control.hpp"

#include <boost/asio/io_context.hpp>
#include <glaze/glaze.hpp>

namespace tfc {

namespace events = sensor::control::events;

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
sensor_control<signal_t, slot_t, sml_t>::sensor_control(asio::io_context& ctx, std::string_view log_key)
    : ctx_{ ctx }, log_key_{ log_key } {
  using enum tfc::operation::mode_e;
  dbus_interface_->initialize();
  operation_mode_.on_enter(running, [this](auto, auto) { this->on_running(); });
  operation_mode_.on_leave(running, [this](auto, auto) { this->on_running_leave(); });
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_idle() {
  stop_motor();
  if (queued_item_) {  // todo test
    logger_.info("Discharge request queued, will propagate event");
    sm_->process_event(events::new_info{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_idle() {}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_awaiting_discharge() {
  auto itm = queued_item_ ? std::move(queued_item_.value()) : ipc::item::make();
  queued_item_ = std::nullopt;
  request_discharge_.async_send(itm.to_json(), [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.info("Failed to send discharge request: {}", err.message());
    }
  });
  if (may_discharge_.value().value_or(false)) {
    sm_->process_event(events::discharge{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_awaiting_discharge() {}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_awaiting_sensor() {
  start_motor();
  discharge_allowance_.async_send(true, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to set discharge active: {}", err.message());
    }
  });
  await_sensor_timer_.expires_after(config_->await_sensor_timeout);
  await_sensor_timer_.async_wait(std::bind_front(&sensor_control::await_sensor_timer_cb, this));
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_awaiting_sensor() {
  stop_motor();
  discharge_allowance_.async_send(false, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to set discharge inactive: {}", err.message());
    }
  });
  await_sensor_timer_.cancel();
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_discharging() {
  start_motor();
  if (!sensor_.value().value_or(false)) {
    sm_->process_event(events::sensor_inactive{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_discharging() {}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_discharge_delayed() {
  if (discharge_timer_) {
    logger_.warn("You are screwed, this is not supposed to happen.");
  }
  if (config_->discharge_timeout) {
    logger_.trace("Starting discharge delay timer for: {}", config_->discharge_timeout.value());
    discharge_timer_.emplace(ctx_);
    discharge_timer_->expires_after(config_->discharge_timeout.value());
    discharge_timer_->async_wait(std::bind_front(&sensor_control::discharge_timer_cb, this));
  } else {
    logger_.info("Discharge delay not configured but called, will transition to idle immediately");
    sm_->process_event(events::complete{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_discharge_delayed() {
  if (discharge_timer_) {
    discharge_timer_->cancel();
    discharge_timer_.reset();
  }
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_discharging_allow_input() {
  discharge_allowance_.async_send(true, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to set discharge active: {}", err.message());
    }
  });
  if (!sensor_.value().value_or(false)) {
    sm_->process_event(events::sensor_inactive{});
  }
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_discharging_allow_input() {
  // next state will set discharge allowance to false
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::on_sensor(bool new_value) {
  if (new_value) {
    sm_->process_event(events::sensor_active{});
  } else {
    sm_->process_event(events::sensor_inactive{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::on_discharge_request(std::string const& new_value) {
  if (auto const err{ glz::validate_json(new_value) }) {
    logger_.info("Discharge request json error: {}", glz::format_error(err, new_value));
    return;
  }
  using ipc::item::item;
  auto parsed_item{ item::from_json(new_value) };
  if (parsed_item.has_value()) {
    set_queued_item(std::move(parsed_item.value()));
    sm_->process_event(events::new_info{});
  } else {
    logger_.info("Discharge request item parse error: {}", glz::format_error(parsed_item.error(), new_value));
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::on_may_discharge(bool new_value) {
  if (new_value) {
    sm_->process_event(events::discharge{});
  }
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::set_queued_item(ipc::item::item&& new_value) {
  // todo I think we need to handle two items in queue !!!!!
  if (queued_item_) {
    logger_.info("Overwriting queued item with id: {}", queued_item_->id());
  }
  queued_item_ = std::move(new_value);
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::await_sensor_timer_cb(const std::error_code& err) {
  if (err) {
    logger_.trace("Await sensor timer error: {}\n", err.message());
    return;
  }
  sm_->process_event(events::await_sensor_timeout{});
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::discharge_timer_cb(std::error_code const& err) {
  if (err) {
    logger_.info("Discharge timer error: {}\n The state machine will continue even though the timer error occurred",
                 err.message());
  }
  discharge_timer_.reset();
  sm_->process_event(events::complete{});
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::on_running() {
  sm_->process_event(events::start{});
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::on_running_leave() {
  sm_->process_event(events::stop{});
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::stop_motor() {
  motor_percentage_.async_send(0.0, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to stop motor: {}", err.message());
    }
  });
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::start_motor() {
  // todo should run speed be configurable in this process?
  // or more generically in ethercat process
  motor_percentage_.async_send(config_->run_speed.numerical_value_, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to start motor: {}", err.message());
    }
  });
}

}  // namespace tfc
