#pragma once

#include <cstddef>

#include <boost/asio.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include "state_machine.hpp"
#include "state_machine_owner.hpp"

namespace tfc::operation {

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
state_machine_owner<signal_t, slot_t, sml_t>::state_machine_owner(asio::io_context& ctx,
                                                                  std::shared_ptr<sdbusplus::asio::connection> conn)
    : ctx_{ ctx }, dbus_{ std::move(conn) },
      states_{ std::make_shared<state_machine_t>(detail::state_machine<state_machine_owner>{ *this }, sml_interface_) } {
  auto print_error = [&](std::string signal_name) {
    return [this, signal_name](std::error_code const& err, std::size_t) {
      if (err) {
        logger_.info("Unable to send {} signal false, error: {}", signal_name, err.message());
      }
    };
  };

  starting_.async_send(false, print_error("starting"));
  running_.async_send(false, print_error("running"));
  stopping_.async_send(false, print_error("stopping"));
  cleaning_.async_send(false, print_error("cleaning"));
  emergency_out_.async_send(false, print_error("emergency_out"));
  fault_out_.async_send(false, print_error("fault_out"));
  mode_str_.async_send("", print_error("mode_str"));
  stop_reason_str_.async_send("", print_error("stop_reason_str"));

  dbus_interface_->initialize();
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
auto state_machine_owner<signal_t, slot_t, sml_t>::set_mode(tfc::operation::mode_e new_mode) -> std::error_code {
  logger_.trace("Got new mode: {}", enum_name(new_mode));

  bool handled{};
  switch (new_mode) {
    using enum mode_e;
    case unknown:
      break;
    case stopping:
    case stopped:
      handled = states_->process_event(detail::events::set_stopped{});
      break;
    case starting:
    case running:
      handled = states_->process_event(detail::events::set_starting{});
      break;
    case cleaning:
      handled = states_->process_event(detail::events::set_cleaning{});
      break;
    case emergency:
      handled = states_->process_event(detail::events::set_emergency{});
      break;
    case fault:
      handled = states_->process_event(detail::events::set_fault{});
      break;
    case maintenance:
      handled = states_->process_event(detail::events::set_maintenance{});
      break;
  }

  if (!handled) {
    return std::make_error_code(std::errc::invalid_argument);
  }
  return {};
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_stopped() {
  stopped_.async_send(true, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send stopped signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_stopped() {
  stopped_.async_send(false, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send stopped signal false, error: {}", err.message());
  });
  stop_reason_str_.async_send(std::string{ "" }, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send stop reason str: none, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_starting() {
  if (config_->startup_time) {
    auto timer{ std::make_shared<asio::steady_timer>(ctx_) };
    timer->expires_after(config_->startup_time.value());
    // todo we should cancel timer if some other event happens
    timer->async_wait([this, timer](std::error_code const& err) { this->on_starting_timer_expired(err); });
  }
  starting_.async_send(true, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send starting signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_starting() {
  starting_.async_send(false, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send starting signal false, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_running() {
  running_.async_send(true, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send running signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_running() {
  running_.async_send(false, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send running signal false, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_stopping() {
  if (config_->stopping_time.has_value()) {
    auto timer{ std::make_shared<asio::steady_timer>(ctx_) };
    timer->expires_after(config_->stopping_time.value());
    // todo we should cancel timer if some other event happens
    timer->async_wait([this, timer](std::error_code const& err) { this->on_stopping_timer_expired(err); });
  }
  stopping_.async_send(true, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send stopping signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_stopping() {
  stopping_.async_send(false, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send stopping signal false, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_cleaning() {
  cleaning_.async_send(true, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send cleaning signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_cleaning() {
  cleaning_.async_send(false, [this](auto err, auto) {
    if (err)
      logger_.info("Unable to send cleaning signal false, error: {}", err.message());
  });
}
// State transition callbacks
// will be used when needed
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_emergency() {
  emergency_out_.async_send(true, [this](std::error_code const& err, std::size_t) {
    logger_.info("Unable to send emergency signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_emergency() {
  emergency_out_.async_send(false, [this](std::error_code const& err, std::size_t) {
    logger_.info("Unable to send emergency signal false, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_fault() {
  fault_out_.async_send(true, [this](std::error_code const& err, std::size_t) {
    logger_.info("Unable to send fault signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_fault() {
  fault_out_.async_send(false, [this](std::error_code const& err, std::size_t) {
    logger_.info("Unable to send fault signal true, error: {}", err.message());
  });
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::enter_maintenance() {}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::leave_maintenance() {}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::transition(mode_e new_mode, mode_e old_mode) {
  if (on_new_state_) {
    std::invoke(on_new_state_, new_mode, old_mode);
  }
  mode_.async_send(std::to_underlying(new_mode), [this, new_mode](auto err, auto) {
    if (err)
      logger_.info("Unable to send mode: {}, error: {}", enum_name(new_mode), err.message());
  });
  mode_str_.async_send(std::string{ enum_name(new_mode) }, [this, new_mode](auto err, auto) {
    if (err)
      logger_.info("Unable to send mode str: {}, error: {}", enum_name(new_mode), err.message());
  });
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::starting_finished_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::starting_finished{});
  }
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::stopping_finished_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::stopping_finished{});
  }
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::running_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::run_button{});
  }
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::cleaning_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::cleaning_button{});
  }
}

// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::maintenance_new_state(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::maintenance_button{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::emergency(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::emergency_on{});
  } else {
    states_->process_event(detail::events::emergency_off{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::fault(bool new_state) {
  if (new_state) {
    states_->process_event(detail::events::fault_on{});
  } else {
    states_->process_event(detail::events::fault_off{});
  }
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::on_starting_timer_expired(std::error_code const& err) {
  if (err) {
    return;
  }
  this->states_->process_event(detail::events::starting_timeout{});
}
// clang-format off
template <template <typename, typename> typename signal_t, template <typename, typename> typename slot_t, template <typename, typename...> typename sml_t>
// clang-format on
void state_machine_owner<signal_t, slot_t, sml_t>::on_stopping_timer_expired(const std::error_code& err) {
  if (err) {
    return;
  }
  this->states_->process_event(detail::events::stopping_timeout{});
}

}  // namespace tfc::operation
