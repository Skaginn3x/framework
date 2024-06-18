#pragma once

#include <tfc/snitch/common.hpp>
#include <tfc/dbus/sdbusplus_fwd.hpp>

namespace tfc::snitch::detail {

class dbus_client {
public:
  dbus_client(std::shared_ptr<sdbusplus::asio::connection> conn) : dbus_{ std::move(conn) } {}

  void register_alarm(std::string_view alarm_name, level lvl, bool ackable);
  
private:
  std::shared_ptr<sdbusplus::asio::connection> dbus_;
};

} // namespace tfc::snitch::detail
