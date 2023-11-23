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
  if (first_time_) {  // todo test
    start_motor();
  } else {
    stop_motor();
  }
  if (queued_item_) {  // todo test
    logger_.info("Discharge request queued, will propagate event");
    sm_->process_event(events::new_info{});
  } else if (sensor_.value().value_or(false)) {
    sm_->process_event(events::sensor_active{});  // todo test
  } else {
    idle_.async_send(true, [this](auto const& err, std::size_t) {  // todo test
      if (err) {
        this->logger_.error("Failed to set idle: {}", err.message());
      }
    });
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_idle() {
  if (first_time_) {
    first_time_ = false;
    stop_motor();
  }
  idle_.async_send(false, [this](auto const& err, std::size_t) {  // todo test
    if (err) {
      this->logger_.error("Failed to set idle: {}", err.message());
    }
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_awaiting_discharge() {
  auto itm = awaiting_sensor_item_ ? std::move(awaiting_sensor_item_.value()) : ipc::item::make();
  awaiting_sensor_item_ = std::nullopt;
  request_discharge_.async_send(itm.to_json(), [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.info("Failed to send discharge request: {}", err.message());
    }
  });
  if (may_discharge_.value().value_or(false)) {
    sm_->process_event(events::discharge{});
  }
  if (!sensor_.value().value_or(false)) {
    sm_->process_event(events::sensor_inactive{}); // Handle if stopping system and removing tub then start system again
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
  logger_.trace("Entering awaiting sensor, value of sensor is: {}", sensor_.value().value_or(false));
  if (awaiting_sensor_item_) {
    logger_.info("I am sorry, won't discard awaiting sensor item with id: {}", awaiting_sensor_item_->id());
  } else {
    awaiting_sensor_item_ = queued_item_ ? std::move(queued_item_.value()) : ipc::item::make();
    queued_item_ = std::nullopt;
  }
  // Make sure we don't already have something blocking the sensor
  if (sensor_.value().value_or(false)) {
    sm_->process_event(events::sensor_active{});  // todo test
    return;
  }
  start_motor();
  pulse_discharge_allowance();
  await_sensor_timer_.expires_after(config_->await_sensor_timeout);
  await_sensor_timer_.async_wait(std::bind_front(&sensor_control::await_sensor_timer_cb, this));
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_awaiting_sensor() {
  stop_motor();
  if (!config_->discharge_allowance_pulse) {
    discharge_allowance_.async_send(false, [this](auto const& err, std::size_t) {
      if (err) {
        this->logger_.warn("Failed to set discharge allowance: {}", err.message());
      }
    });
  }
  await_sensor_timer_.cancel();
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_discharging() {
  start_motor();
  discharge_active_.async_send(true, [this](auto const& err, std::size_t) {  // todo test
    if (err) {
      this->logger_.error("Failed to set discharge active: {}", err.message());
    }
  });
  if (!sensor_.value().value_or(false)) {
    sm_->process_event(events::sensor_inactive{});
  } else if (queued_item_) {
    sm_->process_event(events::new_info{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_discharging() {
  discharge_active_.async_send(false, [this](auto const& err, std::size_t) {  // todo test
    if (err) {
      this->logger_.error("Failed to set discharge active: {}", err.message());
    }
  });
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::enter_uncontrolled_discharge() {}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void sensor_control<signal_t, slot_t, sml_t>::leave_uncontrolled_discharge() {}

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
  pulse_discharge_allowance();
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
  // queued_item_.reset();  // todo test
  awaiting_sensor_item_.reset();  // todo test
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
  // will be implemented in motor library further on
  motor_percentage_.async_send(config_->run_speed.numerical_value_, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to start motor: {}", err.message());
    }
  });
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
// todo test
void sensor_control<signal_t, slot_t, sml_t>::pulse_discharge_allowance() {
  discharge_allowance_.async_send(true, [this](auto const& err, std::size_t) {
    if (err) {
      logger_.warn("Failed to send discharge allowance: {}", err.message());
    }
  });
  if (config_->discharge_allowance_pulse) {
    if (discharge_allowance_pulse_timer_) {
      discharge_allowance_pulse_timer_.reset();
    }
    discharge_allowance_pulse_timer_ = asio::steady_timer{ ctx_ };
    discharge_allowance_pulse_timer_->expires_after(config_->discharge_allowance_pulse.value());
    discharge_allowance_pulse_timer_->async_wait([this](std::error_code const& err) {
      if (err) {
        logger_.info("Discharge allowance pulse timer error: {}", err.message());
        return;
      }
      discharge_allowance_.async_send(false, [this](std::error_code const& err2, std::size_t) {
        logger_.warn("Failed to send discharge allowance: {}", err2.message());
      });
    });
  }
}

}  // namespace tfc
