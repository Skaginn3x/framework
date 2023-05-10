#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

#include <tfc/dbus/sdbusplus_fwd.hpp>

namespace boost::asio {
class io_context;
}

namespace tfc::confman::detail {

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
  std::unique_ptr<sdbusplus::asio::object_server, std::function<void(sdbusplus::asio::object_server*)>> dbus_object_server_;
  std::unique_ptr<sdbusplus::asio::dbus_interface> dbus_interface_{};
};

}  // namespace tfc::confman::detail
