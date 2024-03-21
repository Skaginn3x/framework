#pragma once

#include <memory>

#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/dbus/sdbusplus_fwd.hpp>

namespace tfc::confman::detail {

class stub_config_dbus_client : public config_dbus_client {
public:
  stub_config_dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn,
                          std::string_view,
                          value_call_t&&,
                          schema_call_t&&,
                          change_call_t&&)
      : config_dbus_client{ conn } {}
};

}  // namespace tfc::confman::detail
