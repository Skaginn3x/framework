#include <tfc/ec/devices/beckhoff/EL2xxx.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>

namespace tfc::ec::devices::beckhoff {

template <typename manager_client_type, size_t size, uint32_t pc, tfc::stx::basic_fixed_string name>
el2xxx<manager_client_type, size, pc, name>::el2xxx(asio::io_context& ctx, manager_client_type& client, uint16_t slave_index)
    : base(slave_index) {
  for (size_t i = 0; i < size; i++) {
    bool_receivers_.emplace_back(std::make_unique<tfc::ipc::slot<ipc::details::type_bool, manager_client_type>>(
        ctx, client, fmt::format("{}.slave{}.out{}", name.view(), slave_index, i),
        std::bind_front(&el2xxx::set_output, this, i)));
  }
}
template <typename manager_client_type, size_t size, uint32_t pc, tfc::stx::basic_fixed_string name>
void el2xxx<manager_client_type, size, pc, name>::process_data(std::span<std::byte>, std::span<std::byte> output) noexcept {
  output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0xff);
  if constexpr (size > 8) {
    output[1] = static_cast<std::byte>(output_states_.to_ulong() >> 8);
  }
}

using imc = tfc::ipc_ruler::ipc_manager_client;
template class el2xxx<imc, el2004<imc>::size_v, el2004<imc>::product_code, el2004<imc>::name_v>;
template class el2xxx<imc, el2008<imc>::size_v, el2008<imc>::product_code, el2008<imc>::name_v>;
template class el2xxx<imc, el2809<imc>::size_v, el2809<imc>::product_code, el2809<imc>::name_v>;

}  // namespace tfc::ec::devices::beckhoff

// todo should we?
//#ifdef TFC_TESTING
#include <tfc/ipc/details/dbus_server_iface_mock.hpp>

namespace tfc::ec::devices::beckhoff {
using imc_m = tfc::ipc_ruler::ipc_manager_client_mock;
template class el2xxx<imc_m, el2004<imc_m>::size_v, el2004<imc_m>::product_code, el2004<imc_m>::name_v>;
template class el2xxx<imc_m, el2008<imc_m>::size_v, el2008<imc_m>::product_code, el2008<imc_m>::name_v>;
template class el2xxx<imc_m, el2809<imc_m>::size_v, el2809<imc_m>::product_code, el2809<imc_m>::name_v>;
}

//#endif
