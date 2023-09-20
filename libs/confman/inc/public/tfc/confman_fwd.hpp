#pragma once

namespace tfc::confman {

namespace detail {
class config_dbus_client;
}

template <typename storage_t>
class file_storage;

template <typename config_storage_t,
          typename file_storage_t = file_storage<config_storage_t>,
          typename config_dbus_client_t = detail::config_dbus_client>
class config;

}  // namespace tfc::confman
