#pragma once
#include <memory>

#include <gmock/gmock.h>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/confman/detail/config_dbus_client.hpp>

namespace tfc::confman::detail {

class mock_config_dbus_client : public config_dbus_client {
public:
  mock_config_dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view, value_call_t&&, schema_call_t&&, change_call_t&&)
      : config_dbus_client{ conn } {}
  MOCK_METHOD((void), set, (std::string && prop), (const));  // NOLINT
};

}  // namespace tfc::confman::detail
