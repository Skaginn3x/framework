#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>
#include <system_error>
#include <expected>

#include <glaze/core/context.hpp>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/dbus/string_maker.hpp>

namespace boost::asio {
class io_context;
}

namespace tfc::confman::detail {

namespace asio = boost::asio;

namespace dbus {
static constexpr std::string_view property_value_name{ "Value" };
static constexpr std::string_view property_schema_name{ "Schema" };
static constexpr std::string_view intent_name{ "Config" };
static constexpr std::string_view interface {
  tfc::dbus::const_dbus_name<intent_name>
};
}  // namespace dbus

class config_dbus_client {
public:
  using dbus_connection_t = std::shared_ptr<sdbusplus::asio::connection>;
  using interface_t = std::shared_ptr<sdbusplus::asio::dbus_interface>;

  /// \brief Empty constructor
  /// \note Should only be used for testing !!!
  explicit config_dbus_client(dbus_connection_t);

  using value_call_t = std::function<std::expected<std::string, glz::error_ctx>()>;
  using schema_call_t = std::function<std::string()>;
  using change_call_t = std::function<std::error_code(std::string_view)>;
  /// \brief make dbus client using given dbus connection
  /// Create a property named `config` using the given `key` to make interface name
  config_dbus_client(dbus_connection_t conn, std::string_view key, value_call_t&&, schema_call_t&&, change_call_t&&);

  void set(std::string&&) const;

  void initialize();

  [[nodiscard]] auto get_io_context() const noexcept -> asio::io_context&;

private:
  std::filesystem::path interface_path_{};
  std::string const interface_name_{ dbus::interface };
  std::string const value_property_name_{ dbus::property_value_name };
  std::string const schema_property_name_{ dbus::property_schema_name };
  value_call_t value_call_{};
  schema_call_t schema_call_{};
  change_call_t change_call_{};
  dbus_connection_t dbus_connection_{};
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
};

}  // namespace tfc::confman::detail
