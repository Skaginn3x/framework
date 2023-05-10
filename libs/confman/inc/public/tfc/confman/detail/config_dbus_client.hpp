#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/stx/to_tuple.hpp>

namespace boost::asio {
class io_context;
}

namespace tfc::confman::detail {

struct config_property {
  // THIS SHOULD BE string_view, sdbusplus does not support it as std::string_view is not convertible to char const*
  std::string value{};
  std::string schema{};
  static constexpr auto dbus_reflection{ [](auto&& self) {
    return tfc::stx::to_tuple(std::forward<decltype(self)>(self));
  } };
};

[[maybe_unused]] static auto operator==(config_property const& lhs, config_property const& rhs) noexcept -> bool {
  return lhs.value == rhs.value && lhs.schema == rhs.schema;
}

class config_dbus_client {
public:
  using value_call_t = std::function<std::string()>;
  using schema_call_t = std::function<std::string()>;
  using change_call_t = std::function<std::error_code(std::string_view)>;
  config_dbus_client(boost::asio::io_context& ctx, std::string_view key, value_call_t&&, schema_call_t&&, change_call_t&&);

private:
  std::filesystem::path interface_path_{};
  std::string interface_name_{};
  std::shared_ptr<sdbusplus::asio::connection> dbus_connection_{};
  std::unique_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
};

}  // namespace tfc::confman::detail
