#pragma once

#include <tfc/confman/detail/config_dbus_client.hpp>

namespace tfc::stubs::confman::detail {

class config_dbus_client : public tfc::confman::detail::config_dbus_client {
  config_dbus_client(boost::asio::io_context& ctx, std::string_view, value_call_t&&, schema_call_t&&, change_call_t&&)
      : tfc::confman::detail::config_dbus_client(ctx){};
};

}  // namespace tfc::stubs::confman::detail
