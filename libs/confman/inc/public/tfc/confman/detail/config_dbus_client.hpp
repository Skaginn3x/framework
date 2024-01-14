#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>
#include <system_error>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/dbus/string_maker.hpp>
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
static constexpr std::string_view config_path{ "Config" };
static constexpr std::string_view path{ tfc::dbus::const_dbus_path<config_path> };
}  // namespace dbus

class config_dbus_client {
public:
  using dbus_connection_t = std::shared_ptr<sdbusplus::asio::connection>;
  using interface_t = std::shared_ptr<sdbusplus::asio::dbus_interface>;

  /// \brief Empty constructor
  /// \note Should only be used for testing !!!
  explicit config_dbus_client(asio::io_context& ctx);

  /// \brief Empty constructor
  /// \note Should only be used for testing !!!
  explicit config_dbus_client(dbus_connection_t);

  using value_call_t = std::function<std::string()>;
  using schema_call_t = std::function<std::string()>;
  using change_call_t = std::function<std::error_code(std::string_view)>;
  /// \brief make dbus client using io_context
  /// Create a new dbus connection with a property named `config` using the given `key` to make interface name
  config_dbus_client(asio::io_context& ctx, std::string_view key, value_call_t&&, schema_call_t&&, change_call_t&&);
  /// \brief make dbus client using given dbus connection
  /// Create a property named `config` using the given `key` to make interface name
  config_dbus_client(dbus_connection_t conn, std::string_view key, value_call_t&&, schema_call_t&&, change_call_t&&);
  /// \brief make dbus client using given interface
  /// Create a property with the given `key`
  config_dbus_client(interface_t intf, std::string_view key, value_call_t&&, schema_call_t&&, change_call_t&&);

  void set(config_property&&) const;

  void initialize();

  [[nodiscard]] auto get_io_context() const noexcept -> asio::io_context&;

  [[nodiscard]] auto get_dbus_interface_name() const -> std::string;

private:
  std::filesystem::path interface_path_{};
  std::string interface_name_{};
  std::string property_name_{ dbus::property_name };
  value_call_t value_call_{};
  schema_call_t schema_call_{};
  change_call_t change_call_{};
  dbus_connection_t dbus_connection_{};
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
};

}  // namespace tfc::confman::detail
