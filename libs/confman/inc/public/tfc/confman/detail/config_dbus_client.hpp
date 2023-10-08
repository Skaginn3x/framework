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

namespace asio = boost::asio;

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

namespace dbus {
static constexpr std::string_view property_name{ "config" };
}

class config_dbus_client {
public:
  using dbus_connection_t = std::shared_ptr<sdbusplus::asio::connection>;
  using interface_t = std::shared_ptr<sdbusplus::asio::dbus_interface>;

  /// \brief Empty constructor
  /// \note Should only be used for testing !!!
  explicit config_dbus_client(asio::io_context& ctx);

  using value_call_t = std::function<std::string()>;
  using schema_call_t = std::function<std::string()>;
  using change_call_t = std::function<std::error_code(std::string_view)>;
  config_dbus_client(asio::io_context& ctx, std::string_view key, value_call_t&&, schema_call_t&&, change_call_t&&);
  config_dbus_client(dbus_connection_t conn, std::string_view key, value_call_t&&, schema_call_t&&, change_call_t&&);
  config_dbus_client(interface_t intf, value_call_t&&, schema_call_t&&, change_call_t&&);

  void set(config_property&&) const;

  void initialize();

private:
  std::filesystem::path interface_path_{};
  std::string interface_name_{};
  value_call_t value_call_{};
  schema_call_t schema_call_{};
  change_call_t change_call_{};
  dbus_connection_t dbus_connection_{};
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
};

}  // namespace tfc::confman::detail
