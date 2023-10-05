#pragma once

#include <utility>

#include <fmt/format.h>
#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc/details/dbus_slot.hpp>
#include <tfc/stx/concepts.hpp>

namespace tfc::ipc::details {

namespace asio = boost::asio;

namespace dbus::tags {
static constexpr std::string_view value{ "Value" };
static constexpr std::string_view slot{ "Slot" };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<slot> };
}

template <typename slot_value_t>
class dbus_slot {
public:
  using value_t = slot_value_t;

  explicit dbus_slot(asio::io_context& ctx, auto&& value_getter)
      : dbus_slot(std::make_shared<sdbusplus::asio::connection>(ctx), std::forward<decltype(value_getter)>(value_getter)) {}
  explicit dbus_slot(std::shared_ptr<sdbusplus::asio::connection> conn, auto&& value_getter)
      : conn_{ std::move(conn) }, value_getter_{ std::forward<decltype(value_getter)>(value_getter) } {}
  asio::io_context& io_context() const noexcept { return conn_->get_io_context(); }
  std::shared_ptr<sdbusplus::asio::connection> connection() const noexcept { return conn_; }
  void initialize(std::string_view slot_name) {
    interface_ = std::make_unique<sdbusplus::asio::dbus_interface>(
        conn_, std::string{ dbus::tags::path }, tfc::dbus::make_dbus_name(fmt::format("{}.{}", slot_name, dbus::tags::value)));
    interface_->register_property_r<value_t>(std::string{ dbus::tags::value }, sdbusplus::vtable::property_::emits_change,
                                             [this]([[maybe_unused]] value_t& old_value) {
                                               if (auto current_value = value_getter_(); current_value.has_value()) {
                                                 return current_value.value();
                                               }
                                               return value_t{};
                                             });
    interface_->initialize();
    conn_->request_name(tfc::dbus::make_dbus_name(fmt::format("{}._slot_", slot_name)).c_str());
  }

  void emit_value(value_t const& value) {
    if (interface_) {
      interface_->set_property(std::string{ dbus::tags::value }, value);
    }
  }

private:
  std::shared_ptr<sdbusplus::asio::connection> conn_;
  std::unique_ptr<sdbusplus::asio::dbus_interface, std::function<void(sdbusplus::asio::dbus_interface*)>> interface_{};
  std::function<std::optional<value_t> const&()> value_getter_{};
};

}  // namespace tfc::ipc::details
