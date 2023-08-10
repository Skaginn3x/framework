#pragma once

#include <functional>

#include <boost/asio/io_context.hpp>

#include <tfc/ipc.hpp>
#include <tfc/ipc/item.hpp>
#include <tfc/logger.hpp>

namespace tfc {

namespace asio = boost::asio;

class sensor_control {
public:
  explicit sensor_control(asio::io_context&);

private:
  void on_sensor(bool new_value);
  void on_discharge_request(std::string const& new_value);
  void on_may_discharge(bool new_value);

  asio::io_context& ctx_;
  ipc_ruler::ipc_manager_client ipc_client_{ ctx_ };
  ipc::bool_slot sensor_{ ctx_, ipc_client_, "sensor", "Sensor input at the end of my conveyor",
                          std::bind_front(&sensor_control::on_sensor, this) };
  ipc::json_signal request_discharge_{ ctx_, ipc_client_, "request_discharge", "Ask for release of item downstream" };
  ipc::json_slot discharge_request_{ ctx_, ipc_client_, "discharge_request", "Get discharge request from upstream",
                                     std::bind_front(&sensor_control::on_discharge_request, this) };
  ipc::bool_signal discharge_active_{ ctx_, ipc_client_, "discharge_active",
                                      "Let downstream know that it can discharge onto me" };
  ipc::bool_slot may_discharge_{ ctx_, ipc_client_, "may_discharge", "Get acknowledgement of discharging my item",
                                 std::bind_front(&sensor_control::on_may_discharge, this) };
  ipc::double_signal motor_percentage_{ ctx_, ipc_client_, "motor_percentage", "Motor freq output, stopped when inactive." };

  ipc::item::item queued_item_{};
  logger::logger logger_{ "sensor_control" };
};

}  // namespace tfc
