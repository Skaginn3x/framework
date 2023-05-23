#pragma once
// todo rename to remote operation?
#include <concepts>

#include <glaze/glaze.hpp>

#include <tfc/dbus/sdbusplus_fwd.hpp>

namespace tfc::confman {

[[maybe_unused]] void set_config_impl(sdbusplus::asio::connection& dbus,
                                      std::string_view key,
                                      std::string_view value,
                                      std::function<void(std::error_code)>);

static void set_config(sdbusplus::asio::connection& dbus,
                       std::string_view key,
                       auto&& storage,
                       std::invocable<std::error_code> auto&& handler) {
  return set_config_impl(dbus, key, glz::write_json(std::forward<decltype(storage)>(storage), std::forward<decltype(handler)>(handler));
}

// todo get_config

}  // namespace tfc::confman
