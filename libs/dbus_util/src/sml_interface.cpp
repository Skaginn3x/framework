#include "tfc/dbus/sml_interface.hpp"

#include <sdbusplus/asio/object_server.hpp>

namespace tfc::dbus::sml {
namespace tags {
static constexpr std::string_view state{ "State" };
static constexpr std::string_view last_state{ "LastState" };
static constexpr std::string_view transition{ "Event" };
static constexpr std::string_view state_machine{ "StateMachine" };
}  // namespace tags

namespace detail {

interface_impl::interface_impl(std::shared_ptr<sdbusplus::asio::dbus_interface> interface) : dbus_interface_{ interface } {
  dbus_interface_->register_property_r<std::string>(
      std::string{ tags::state }, sdbusplus::vtable::property_::emits_change,
      [this]([[maybe_unused]] std::string& old_value) -> std::string { return destination_state_; });
  dbus_interface_->register_property_r<std::string>(
      std::string{ tags::last_state }, sdbusplus::vtable::property_::emits_change,
      [this]([[maybe_unused]] std::string& old_value) -> std::string { return source_state_; });
  dbus_interface_->register_property_r<std::string>(
      std::string{ tags::transition }, sdbusplus::vtable::property_::emits_change,
      [this]([[maybe_unused]] std::string& old_value) -> std::string { return event_; });
  dbus_interface_->register_property_r<std::string>(
      std::string{ tags::state_machine }, sdbusplus::vtable::property_::emits_change,
      [this]([[maybe_unused]] std::string& old_value) -> std::string { return state_machine_dot_formatted_; });
}
void interface_impl::on_state_change(std::string_view source_state,
                                     std::string_view destination_state,
                                     std::string_view event) {
  source_state_ = source_state;
  destination_state_ = destination_state;
  event_ = event;
  dbus_interface_->set_property(std::string{ tags::last_state }, source_state_);
  dbus_interface_->set_property(std::string{ tags::state }, destination_state_);
  dbus_interface_->set_property(std::string{ tags::transition }, event_);
}
void interface_impl::dot_format(std::string_view state_machine) {
  state_machine_dot_formatted_ = state_machine;
  dbus_interface_->set_property(std::string{ tags::state_machine }, state_machine_dot_formatted_);
}

}  // namespace detail
}  // namespace tfc::dbus::sml
