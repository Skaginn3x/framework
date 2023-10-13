#pragma once

#include <functional>
#include <optional>

#include <boost/sml.hpp>

#include <tfc/ipc.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/ipc/item.hpp>
#include <tfc/logger.hpp>
#include <tfc/sml_logger.hpp>
#include <tfc/utils/asio_fwd.hpp>

#include "state_machine.hpp"

namespace tfc {

namespace asio = boost::asio;

template <template <typename, typename manager_client_t = ipc_ruler::ipc_manager_client&> typename signal_t = ipc::signal,
          template <typename, typename manager_client_t = ipc_ruler::ipc_manager_client&> typename slot_t = ipc::slot,
          template <typename, typename...> typename sml_t = boost::sml::sm>
class sensor_control {
public:
  explicit sensor_control(asio::io_context&);

  void enter_idle();
  void leave_idle();
  void enter_awaiting_discharge();
  void leave_awaiting_discharge();
  void enter_awaiting_sensor();
  void leave_awaiting_sensor();
  void enter_discharging();
  void leave_discharging();

  // accessors for testing purposes
  [[nodiscard]] auto motor_signal() const noexcept -> auto const& { return motor_percentage_; }
  [[nodiscard]] auto discharge_signal() const noexcept -> auto const& { return request_discharge_; }
  [[nodiscard]] auto discharge_allowance_signal() const noexcept -> auto const& { return discharge_allowance_; }
  [[nodiscard]] auto may_discharge_slot() const noexcept -> auto const& { return may_discharge_; }
  [[nodiscard]] auto state_machine() const noexcept -> auto const& { return sm_; }
  [[nodiscard]] auto queued_item() const noexcept -> auto const& { return queued_item_; }

  void on_sensor(bool new_value);
  void on_discharge_request(std::string const& new_value);
  void on_may_discharge(bool new_value);
  void set_queued_item(ipc::item::item&&);

private:
  void stop_motor();
  void start_motor();

  using bool_signal_t = signal_t<ipc::details::type_bool>;
  using double_signal_t = signal_t<ipc::details::type_double>;
  using json_signal_t = signal_t<ipc::details::type_json>;
  using bool_slot_t = slot_t<ipc::details::type_bool>;
  using json_slot_t = slot_t<ipc::details::type_json>;

  asio::io_context& ctx_;
  ipc_ruler::ipc_manager_client ipc_client_{ ctx_ };
  bool_slot_t sensor_{ ctx_, ipc_client_, "sensor",
                       "Sensor input at the end of my conveyor, item will stop here and wait for discharge",
                       std::bind_front(&sensor_control::on_sensor, this) };
  json_signal_t request_discharge_{ ctx_, ipc_client_, "request_discharge", "Ask for release of item downstream" };
  json_slot_t discharge_request_{ ctx_, ipc_client_, "discharge_request", "Get discharge request from upstream",
                                  std::bind_front(&sensor_control::on_discharge_request, this) };
  bool_signal_t discharge_allowance_{ ctx_, ipc_client_, "discharge_allowance",
                                      "Let upstream know that it can discharge onto me" };
  bool_signal_t discharge_active_{ ctx_, ipc_client_, "discharge_active",
                                      "Discharging item to downstream" };
  bool_slot_t may_discharge_{ ctx_, ipc_client_, "may_discharge", "Get acknowledgement of discharging my item",
                              std::bind_front(&sensor_control::on_may_discharge, this) };
  double_signal_t motor_percentage_{ ctx_, ipc_client_, "motor_percentage", "Motor freq output, stopped when inactive." };

  std::optional<ipc::item::item> queued_item_{ std::nullopt };
  logger::logger logger_{ "sensor_control" };

  using state_machine_t = sml_t<sensor::control::state_machine<sensor_control>, boost::sml::logger<tfc::logger::sml_logger>>;
  std::shared_ptr<state_machine_t> sm_{
    std::make_shared<state_machine_t>(sensor::control::state_machine<sensor_control>{ *this }, tfc::logger::sml_logger{})
  };
};

extern template class sensor_control<>;

}  // namespace tfc
