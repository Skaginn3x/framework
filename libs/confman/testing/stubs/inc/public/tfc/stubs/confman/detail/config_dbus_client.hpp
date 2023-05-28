#pragma once

#include <boost/asio/io_context.hpp>

#include <tfc/confman/detail/config_dbus_client.hpp>

namespace tfc::confman::detail {

class stub_config_dbus_client : public config_dbus_client {
public:
  stub_config_dbus_client(boost::asio::io_context& ctx, std::string_view, value_call_t&&, schema_call_t&&, change_call_t&&)
      : config_dbus_client{ ctx } {}
};

}  // namespace tfc::confman::detail
