#pragma once

#include <memory>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/utils/asio_fwd.hpp>

namespace tfc::ipc::details {

namespace asio = boost::asio;

class dbus_slot {
  explicit dbus_slot(asio::io_context&);
  explicit dbus_slot(std::shared_ptr<sdbusplus::asio::connection> conn);
};

}  // namespace tfc::ipc::details
