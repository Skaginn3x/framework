#include "tfc/dbus/sml_interface.hpp"

#include <sdbusplus/asio/object_server.hpp>

namespace tfc::dbus::sml {
namespace tags {
static constexpr std::string_view state{ "State" };
static constexpr std::string_view last_state{ "LastState" };
static constexpr std::string_view transition{ "Event" };
static constexpr std::string_view dot_format{ "DotFormat" };
}  // namespace tags

namespace detail {

interface_impl::interface_impl(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view unique_key)
    : dbus_interface_{ std::make_shared<sdbusplus::asio::dbus_interface>(conn,
                                                                         dbus::make_dbus_path(unique_key),
                                                                         std::string{ tags::interface }) } {
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
      std::string{ tags::dot_format }, sdbusplus::vtable::property_::emits_change,
      [this]([[maybe_unused]] std::string& old_value) -> std::string { return state_machine_dot_formatted_; });
  dbus_interface_->initialize();
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
  dbus_interface_->set_property(std::string{ tags::dot_format }, state_machine_dot_formatted_);
}

namespace test {
struct test_state {
  static constexpr std::string_view name{ "test_state" };
};
struct invalid_test_state {
  static constexpr std::string_view name1{ "test_state" };
};
static_assert(name_exists<test_state>);
static_assert(!name_exists<invalid_test_state>);
}  // namespace test

}  // namespace detail
}  // namespace tfc::dbus::sml
