#pragma once

#include <boost/sml.hpp>

#include <tfc/ipc_fwd.hpp>
#include <tfc/logger.hpp>
#include <tfc/operation_mode/common.hpp>

import sdbus;
import std;
import asio;

namespace tfc {

namespace operation {
template <template <typename description_t, typename manager_client_t> typename signal_t,
          template <typename description_t, typename manager_client_t>
          typename slot_t,
          template <typename, typename...>
          typename sml_t>
class state_machine_owner;
}  // namespace operation

template <template <typename description_t, typename manager_client_t> typename signal_t = ipc::signal,
          template <typename description_t, typename manager_client_t> typename slot_t = ipc::slot>
class app_operation_mode {
public:
  explicit app_operation_mode(asio::io_context&);

  auto set_mode(tfc::operation::mode_e new_mode) -> void;

private:
  std::shared_ptr<sdbusplus::asio::connection> dbus_;
  std::unique_ptr<sdbusplus::asio::object_server, std::function<void(sdbusplus::asio::object_server*)>> dbus_object_server_;
  std::shared_ptr<sdbusplus::asio::dbus_interface> dbus_interface_;
  using state_machine_owner_t = operation::state_machine_owner<signal_t, slot_t, boost::sml::sm>;
  std::unique_ptr<state_machine_owner_t, std::function<void(state_machine_owner_t*)>> state_machine_;
  tfc::operation::mode_e mode_{ tfc::operation::mode_e::unknown };
  tfc::logger::logger logger_;
};

extern template class app_operation_mode<>;

}  // namespace tfc
