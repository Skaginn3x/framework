#include <filesystem>
#include <string_view>

#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/property.hpp>

#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/confman/remote_change.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/dbus/string_maker.hpp>
import tfc.base;

namespace tfc::confman {

[[maybe_unused]] void set_config_impl(sdbusplus::asio::connection& dbus,
                                      std::string_view key,
                                      std::string_view value,
                                      std::function<void(std::error_code)> handler) {
  auto interface_path{ std::filesystem::path{ tfc::dbus::make_dbus_path("") } /
                       tfc::base::make_config_file_name(key, "").string().substr(1) };
  auto interface_name{ interface_path.string().substr(1) };
  std::replace(interface_name.begin(), interface_name.end(), '/', '.');

  sdbusplus::asio::setProperty<detail::config_property>(
      dbus, interface_name, interface_path.string(), interface_name,
      std::string{ detail::dbus::property_name.data(), detail::dbus::property_name.size() },
      detail::config_property{ std::string{ value.data(), value.size() }, "" }, std::move(handler));
}

}  // namespace tfc::confman
