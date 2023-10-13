#pragma once

#include "sensor_control.hpp"

#include <boost/asio/io_context.hpp>
#include <glaze/glaze.hpp>

namespace tfc {

namespace events = sensor::control::events;

template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
sensor_control<signal_t, slot_t, sml_t>::sensor_control(asio::io_context& ctx) : ctx_{ ctx } {}

template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::enter_idle() {
  stop_motor();
}
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::leave_idle() {}
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
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
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::leave_awaiting_discharge() {}
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::enter_awaiting_sensor() {
  start_motor();
  discharge_allowance_.async_send(true, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to set discharge active: {}", err.message());
    }
  });
}
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::leave_awaiting_sensor() {
  stop_motor();
  discharge_allowance_.async_send(false, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to set discharge inactive: {}", err.message());
    }
  });
}
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::enter_discharging() {
  start_motor();
}
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::leave_discharging() {}

template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::on_sensor(bool new_value) {
  if (new_value) {
    sm_->process_event(events::sensor_active{});
  }
  else {
    sm_->process_event(events::complete{});
  }
}
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
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
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::on_may_discharge(bool new_value) {
  if (new_value) {
    sm_->process_event(events::discharge{});
  }
}

template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::set_queued_item(ipc::item::item&& new_value) {
  if (queued_item_) {
    logger_.info("Overwriting queued item with id: {}", queued_item_->id());
  }
  queued_item_ = std::move(new_value);
}

template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::stop_motor() {
  motor_percentage_.async_send(0.0, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to stop motor: {}", err.message());
    }
  });
}

template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
void sensor_control<signal_t, slot_t, sml_t>::start_motor() {
  // todo should run speed be configurable in this process?
  // or more generically in ethercat process
  motor_percentage_.async_send(100.0, [this](auto const& err, std::size_t) {
    if (err) {
      this->logger_.error("Failed to start motor: {}", err.message());
    }
  });
}

}  // namespace tfc
