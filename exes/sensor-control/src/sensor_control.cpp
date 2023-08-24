#include "sensor_control.hpp"

#include <glaze/glaze.hpp>

namespace tfc {

sensor_control::sensor_control(asio::io_context& ctx) : ctx_{ ctx } {}

void sensor_control::enter_idle() {}
void sensor_control::leave_idle() {}

void sensor_control::on_sensor(bool new_value) {}
void sensor_control::on_discharge_request(std::string const& new_value) {
  if (auto const err{ glz::validate_json(new_value) }) {
    logger_.info("Discharge request json error: {}", glz::format_error(err, new_value));
    queued_item_ = ipc::item::item::make();
    return;
  }
  auto const parsed_item{ glz::read_json<ipc::item::item>(new_value) };
  if (parsed_item.has_value()) {
    queued_item_ = std::move(parsed_item.value());
  } else {
    logger_.info("Discharge request item parse error: {}", glz::format_error(parsed_item.error(), new_value));
    queued_item_ = ipc::item::item::make();
  }
}
void sensor_control::on_may_discharge(bool new_value) {}

}  // namespace tfc
