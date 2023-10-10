#pragma once

#include <utility>

#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc/details/dbus_slot.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::ipc::details {

namespace asio = boost::asio;

namespace dbus::tags {
static constexpr std::string_view value{ "Value" };
static constexpr std::string_view slot{ "Slots" };
static constexpr std::string_view tinker{ "Tinker" };
static constexpr std::string_view type{ "Type" };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<slot> };
}  // namespace dbus::tags

template <typename slot_value_t>
class dbus_slot {
public:
  using value_t = slot_value_t;

  explicit dbus_slot(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view slot_name, auto&& value_getter)
      : interface_{ std::make_shared<sdbusplus::asio::dbus_interface>(conn,
                                                                      std::string{ dbus::tags::path },
                                                                      tfc::dbus::make_dbus_name(slot_name)) },
        value_getter_{ std::forward<decltype(value_getter)>(value_getter) } {}
  void initialize() {
    interface_->register_property_r<value_t>(std::string{ dbus::tags::value }, sdbusplus::vtable::property_::emits_change,
                                             [this]([[maybe_unused]] value_t& old_value) {
                                               if (auto current_value = value_getter_(); current_value.has_value()) {
                                                 return current_value.value();
                                               }
                                               return value_t{};
                                             });
    interface_->register_property_r<std::string>(std::string{ dbus::tags::type }, sdbusplus::vtable::property_::emits_change,
                                             []([[maybe_unused]] std::string& old_value) {
                                               return tfc::json::write_json_schema<value_t>();
                                             });

    interface_->initialize();
  }
  auto interface() const noexcept -> std::shared_ptr<sdbusplus::asio::dbus_interface> { return interface_; }

  void emit_value(value_t const& value) {
    if (interface_) {
      interface_->set_property(std::string{ dbus::tags::value }, value);
    }
  }

  void on_set(tfc::stx::invocable<value_t&&> auto&& callback) {
    interface_->register_method(
        std::string{ dbus::tags::tinker },
        [callb = std::forward<decltype(callback)>(callback)](value_t const& set_value) { callb(value_t{ set_value }); });
  }

private:
  std::shared_ptr<sdbusplus::asio::dbus_interface> interface_{};
  std::function<std::optional<value_t> const&()> value_getter_{};
};

}  // namespace tfc::ipc::details
