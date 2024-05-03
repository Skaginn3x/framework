#include <filesystem>
#include <string_view>

#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/property.hpp>

#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/confman/remote_change.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/progbase.hpp>

namespace tfc::confman {

[[maybe_unused]] void set_config_impl(sdbusplus::asio::connection& dbus,
                                      std::string_view service,
                                      std::string_view key,
                                      std::string_view value,
                                      std::function<void(std::error_code)> handler) {
  auto const interface_path{ tfc::dbus::make_dbus_path(key) };
  std::string const interface_name{ tfc::confman::detail::dbus::interface };
  sdbusplus::asio::setProperty<std::string>(dbus, std::string{ service }, interface_path, interface_name,
                                            std::string{ tfc::confman::detail::dbus::property_value_name },
                                            std::string{ value }, std::move(handler));
}

}  // namespace tfc::confman
