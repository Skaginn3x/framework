#pragma once

#include <functional>
#include <optional>

#include <mp-units/quantity.h>
#include <mp-units/unit.h>
#include <boost/asio/steady_timer.hpp>
#include <boost/sml.hpp>

#include <tfc/dbus/sml_interface.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/ipc/item.hpp>
#include <tfc/logger.hpp>
#include <tfc/operation_mode.hpp>
#include <tfc/utils/asio_fwd.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

#include "state_machine.hpp"

namespace tfc {

namespace asio = boost::asio;

template <template <typename, typename manager_client_t = ipc_ruler::ipc_manager_client&> typename signal_t = ipc::signal,
          template <typename, typename manager_client_t = ipc_ruler::ipc_manager_client&> typename slot_t = ipc::slot,
          template <typename, typename...> typename sml_t = boost::sml::sm>
class sensor_control {
public:
  explicit sensor_control(asio::io_context& ctx, std::string_view log_key);
  explicit sensor_control(asio::io_context& ctx) : sensor_control(ctx, "sensor_control") {}

  struct sensor_control_config {
    std::optional<std::chrono::milliseconds> discharge_timeout{ std::nullopt };
    std::chrono::milliseconds minimum_discharge_duration{ std::chrono::seconds{ 1 } };
    std::chrono::milliseconds await_sensor_timeout{ std::chrono::minutes{ 1 } };
    mp_units::quantity<mp_units::percent, double> run_speed{ 0.0 * mp_units::percent };
    bool run_on_discharge{ true };
    bool allow_item_removal{ false };
    struct glaze {
      using type = sensor_control_config;
      static constexpr std::string_view name{ "tfc::sensor_control" };
      // clang-format off
      static constexpr auto value{ glz::object(
        "discharge delay", &type::discharge_timeout, "Delay after falling edge of sensor to keep output of discharge active high.",
        "minimum discharge duration", &type::minimum_discharge_duration, "Minumum discharge duration, set with relation to item length and conveyor speed.",
        "await sensor timeout", &type::await_sensor_timeout, "Timeout for awaiting sensor input.",
        "run speed", &type::run_speed, json::schema{ .description="Speed of the motor when running.", .minimum=0.0, .maximum=100.0 },
        "run on discharge", &type::run_on_discharge, "Run the motor when discharging an item.",
        "allow item removal", &type::allow_item_removal, "Allow removing item while awaiting discharge"
      ) };
      // clang-format on
    };
  };

  void enter_stopped() {}
  void leave_stopped() {}
  void enter_running() {}
  void leave_running() {}
  void enter_idle();
  void leave_idle();
  void enter_awaiting_discharge();
  void leave_awaiting_discharge();
  void enter_awaiting_sensor();
  void leave_awaiting_sensor();
  void enter_discharging();
  void leave_discharging();
  void enter_uncontrolled_discharge();
  void leave_uncontrolled_discharge();
  void enter_discharge_delayed();
  void leave_discharge_delayed();
  void enter_discharging_allow_input();
  void leave_discharging_allow_input();

  auto config_run_on_discharge() const noexcept -> bool { return config_.value().run_on_discharge; }

  // accessors for testing purposes
  [[nodiscard]] auto motor_signal() const noexcept -> auto const& { return motor_percentage_; }
  [[nodiscard]] auto discharge_signal() const noexcept -> auto const& { return request_discharge_; }
  [[nodiscard]] auto discharge_allowance_signal() const noexcept -> auto const& { return discharge_allowance_; }
  [[nodiscard]] auto discharge_request_slot() const noexcept -> auto const& { return discharge_request_; }
  [[nodiscard]] auto may_discharge_slot() const noexcept -> auto const& { return may_discharge_; }
  [[nodiscard]] auto sensor_slot() const noexcept -> auto const& { return sensor_; }
  [[nodiscard]] auto state_machine() const noexcept -> auto const& { return sm_; }
  [[nodiscard]] auto queued_item() const noexcept -> auto const& { return queued_item_; }
  [[nodiscard]] auto config() noexcept -> auto& { return config_; }

  void on_sensor(bool new_value);
  void on_discharge_request(std::string const& new_value);
  void on_may_discharge(std::string const& new_value);
  void set_queued_item(ipc::item::item&&);

  void await_sensor_timer_cb(std::error_code const&);
  void discharge_timer_cb(std::error_code const&);
  [[nodiscard]] auto using_discharge_delay() const noexcept -> bool { return config_->discharge_timeout.has_value(); }
  [[nodiscard]] auto allow_item_removal() const noexcept -> bool { return config_->allow_item_removal; }
  [[nodiscard]] auto min_discharge_duration_elapsed() const noexcept -> bool { return !min_discharge_timer_is_on_; }

  void on_running();
  void on_running_leave();

private:
  void stop_motor();
  void start_motor();
  void pulse_discharge_allowance(ipc::item::item& itm);
  void start_min_discharge_timer();

  using bool_signal_t = signal_t<ipc::details::type_bool>;
  using double_signal_t = signal_t<ipc::details::type_double>;
  using json_signal_t = signal_t<ipc::details::type_json>;
  using string_signal_t = signal_t<ipc::details::type_string>;
  using bool_slot_t = slot_t<ipc::details::type_bool>;
  using json_slot_t = slot_t<ipc::details::type_json>;
  using string_slot_t = slot_t<ipc::details::type_string>;

  asio::io_context& ctx_;
  std::string_view log_key_;
  ipc_ruler::ipc_manager_client ipc_client_{ ctx_ };
  bool_slot_t sensor_{ ctx_, ipc_client_, "sensor",
                       "Sensor input at the end of my conveyor, item will stop here and wait for discharge",
                       std::bind_front(&sensor_control::on_sensor, this) };
  json_signal_t request_discharge_{ ctx_, ipc_client_, "request_discharge", "Ask for release of item downstream" };
  json_slot_t discharge_request_{ ctx_, ipc_client_, "discharge_request", "Get discharge request from upstream",
                                  std::bind_front(&sensor_control::on_discharge_request, this) };
  bool_signal_t idle_{ ctx_, ipc_client_, "idle", "Indication of not doing anything" };
  string_signal_t discharge_allowance_{ ctx_, ipc_client_, "discharge_allowance_uuid",
                                        "Let upstream know that the uuid can discharge onto me" };
  bool_signal_t discharge_active_{ ctx_, ipc_client_, "discharge_active", "Discharging item to downstream" };
  string_slot_t may_discharge_{ ctx_, ipc_client_, "may_discharge_uuid", "Get acknowledgement of discharging item with uuid",
                                std::bind_front(&sensor_control::on_may_discharge, this) };
  double_signal_t motor_percentage_{ ctx_, ipc_client_, "motor_percentage", "Motor freq output, stopped when inactive." };

  std::optional<ipc::item::item> queued_item_{ std::nullopt };
  std::optional<ipc::item::item> current_item_{ std::nullopt };

  logger::logger logger_{ log_key_ };

  using state_machine_t =
      sml_t<sensor::control::state_machine_operation_mode<sensor_control>, boost::sml::logger<tfc::dbus::sml::interface>>;

  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{ std::make_shared<sdbusplus::asio::dbus_interface>(
      ipc_client_.connection(),
      std::string{ tfc::dbus::sml::tags::path },
      tfc::dbus::make_dbus_name("SensorControl")) };

  dbus::sml::interface sml_interface_ {
    dbus_interface_, fmt::format("sm.{}", log_key_)
  };
  std::shared_ptr<state_machine_t> sm_{ std::make_shared<state_machine_t>(*this, sml_interface_) };

  asio::steady_timer await_sensor_timer_{ ctx_ };
  asio::steady_timer min_discharge_timer_{ ctx_ };
  bool min_discharge_timer_is_on_{ false };
  std::optional<asio::steady_timer> discharge_timer_{ std::nullopt };

  confman::config<sensor_control_config> config_{ ctx_, "sensor_control",
                                                  sensor_control_config{ .discharge_timeout = std::nullopt,
                                                                         .minimum_discharge_duration =
                                                                             std::chrono::seconds{ 1 },
                                                                         .await_sensor_timeout = std::chrono::minutes{ 1 },
                                                                         .run_speed = 100.0 * mp_units::percent,
                                                                         .run_on_discharge = true,
                                                                         .allow_item_removal = false } };
  operation::interface operation_mode_ {
    ctx_, "operation_mode"
  };
  bool first_time_{ true };
};

extern template class sensor_control<>;

}  // namespace tfc
