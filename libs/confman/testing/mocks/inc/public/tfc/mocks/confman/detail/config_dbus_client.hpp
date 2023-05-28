#pragma once

#include <gmock/gmock.h>
#include <boost/asio/io_context.hpp>

#include <tfc/confman/detail/config_dbus_client.hpp>

namespace tfc::confman::detail {

class mock_config_dbus_client : public config_dbus_client {
public:
  mock_config_dbus_client(boost::asio::io_context& ctx, std::string_view, value_call_t&&, schema_call_t&&, change_call_t&&)
      : config_dbus_client{ ctx } {};
  MOCK_METHOD((void), set, (config_property&& prop), (const));       // NOLINT
};

}  // namespace tfc::confman::detail
