#pragma once

#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc/details/dbus_ipc.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/utils/json_schema.hpp>

namespace tfc::ipc::details {

namespace dbus::tags {
static constexpr std::string_view value{ "Value" };
static constexpr std::string_view slot{ "Slot" };
static constexpr std::string_view signal{ "Signal" };
static constexpr std::string_view tinker{ "Tinker" };
static constexpr std::string_view type{ "Type" };
static constexpr std::string_view slot_interface{ tfc::dbus::const_dbus_name<slot> };
static constexpr std::string_view signal_interface{ tfc::dbus::const_dbus_name<signal> };
}  // namespace dbus::tags

enum struct ipc_type_e : std::uint8_t { slot, signal };

template <typename slot_value_t, ipc_type_e type>
class dbus_ipc {
public:
  using value_t = slot_value_t;
  std::string const interface_name{ type == ipc_type_e::signal ? dbus::tags::signal_interface : dbus::tags::slot_interface };

  explicit dbus_ipc(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view key)
      : interface_{
          std::make_shared<sdbusplus::asio::dbus_interface>(conn, tfc::dbus::make_dbus_path(key), interface_name)
        } {}

  dbus_ipc(dbus_ipc const&) = delete;
  dbus_ipc(dbus_ipc&&) noexcept = default;
  auto operator=(dbus_ipc const&) -> dbus_ipc& = delete;
  auto operator=(dbus_ipc&&) noexcept -> dbus_ipc& = default;

  void initialize() {
    interface_->register_signal<value_t>(std::string{ dbus::tags::value });
    interface_->register_property_r<value_t>(std::string{ dbus::tags::value }, sdbusplus::vtable::property_::none,
                                             [this](const auto&) { return value_; });
    interface_->register_property_r<std::string>(
        std::string{ dbus::tags::type }, sdbusplus::vtable::property_::const_, []([[maybe_unused]] std::string& old_value) {
          auto const val{ tfc::json::write_json_schema<value_t>() };
          if (!val) {
            fmt::println(stderr, "Unable to get schema: '{}'", glz::format_error(val.error()));
            return std::string{};
          }
          return val.value();
        });

    interface_->initialize();
  }

  void emit_value(value_t const& value) {
    value_ = value;
    auto message = interface_->new_signal(dbus::tags::value.data());
    message.append(value);
    message.signal_send();
  }

  void on_set(tfc::stx::invocable<value_t&&> auto&& callback) {
    interface_->register_method(
        std::string{ dbus::tags::tinker },
        [callb = std::forward<decltype(callback)>(callback)](value_t const& set_value) { callb(value_t{ set_value }); });
  }

private:
  std::shared_ptr<sdbusplus::asio::dbus_interface> interface_{};
  value_t value_{};
};

}  // namespace tfc::ipc::details
