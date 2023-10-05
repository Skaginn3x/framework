#pragma once

#include <memory>

#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/utils/asio_fwd.hpp>

namespace tfc::ipc::details {

namespace asio = boost::asio;

class dbus_slot {
public:
  explicit dbus_slot(asio::io_context&);
  explicit dbus_slot(std::shared_ptr<sdbusplus::asio::connection> conn);
  asio::io_context& io_context() const noexcept;
  std::shared_ptr<sdbusplus::asio::connection> connection() const noexcept;
  void initialize(std::string_view slot_name);
private:
  std::shared_ptr<sdbusplus::asio::connection> conn_;
  std::unique_ptr<sdbusplus::asio::dbus_interface, std::function<void(sdbusplus::asio::dbus_interface*)>> interface_{};
};

}  // namespace tfc::ipc::details
