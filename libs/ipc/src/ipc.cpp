#include "tfc/ipc.hpp"

#include <tfc/utils/socket.hpp>

namespace tfc::ipc {

auto transmission_base::endpoint() const -> std::string {
  return utils::socket::zmq::ipc_endpoint_str(fmt::format("{}.{}.{}", base::get_exe_name(), base::get_proc_name(), name_));
}

}  // namespace tfc::ipc
