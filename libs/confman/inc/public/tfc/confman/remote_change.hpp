#pragma once
// todo rename to remote operation?
#include <concepts>

#include <glaze/glaze.hpp>

#include <tfc/dbus/sdbusplus_fwd.hpp>

namespace tfc::confman {

[[maybe_unused]] void set_config_impl(sdbusplus::asio::connection& dbus,
                                      std::string_view service,
                                      std::string_view key,
                                      std::string_view value,
                                      std::function<void(std::error_code)>);

template <typename config_storage_t>
[[maybe_unused]] static void set_config(sdbusplus::asio::connection& dbus,
                                        std::string_view service,
                                        std::string_view key,
                                        config_storage_t&& storage,
                                        std::invocable<std::error_code> auto&& handler) {
  auto const write{ glz::write_json(std::forward<config_storage_t>(storage)) };
  if (!write) {
    handler(std::make_error_code(std::errc::invalid_argument));
    return;
  }
  return set_config_impl(dbus, service, key, write.value(),
                         std::forward<decltype(handler)>(handler));
}

// todo get_config

}  // namespace tfc::confman
