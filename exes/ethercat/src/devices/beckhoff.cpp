#include <tfc/ec/devices/beckhoff/EL1xxx.hpp>
#include <tfc/ec/devices/beckhoff/EL2xxx.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>

namespace tfc::ec::devices::beckhoff {

// TODO discuss! I would like to remove the name of the device from the ipc channel name
// Reason being it makes it harder to use the same input/output declaration with recipes of system
// Example, let's say we have a recipe for a system with 2 input devices and 2 output devices
// Let's say we change vendor and the name would change, however, the recipe would still be valid


// el1xxx

template <typename manager_client_type, size_t size, uint32_t pc>
el100x<manager_client_type, size, pc>::el100x(asio::io_context& ctx, manager_client_type& client, const uint16_t slave_index)
    : base(slave_index) {
  for (size_t i = 0; i < size; i++) {
    transmitters_.emplace_back(
        std::make_unique<signal_t>(ctx, client, fmt::format("EL100{}.{}.in.{}", size, slave_index, i), "Digital input"));
  }
}

template <typename manager_client_type, size_t size, uint32_t pc>
void el100x<manager_client_type, size, pc>::process_data(std::span<std::byte> input, std::span<std::byte>) noexcept {
  static_assert(size <= 8);
  std::bitset<size> const in_bits(static_cast<uint8_t>(input[0]));
  for (size_t i = 0; i < size; i++) {
    bool const value = in_bits.test(i);
    if (value != last_values_[i]) {
      transmitters_[i]->async_send(value, [this](std::error_code error, size_t) {
        if (error) {
          logger_.error("Ethercat EL110x, error transmitting : {}", error.message());
        }
      });
    }
    last_values_[i] = value;
  }
}

// el2xxx

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

// symbol export for ipc_manager_client

using imc = tfc::ipc_ruler::ipc_manager_client;

template class el100x<imc, el1002<imc>::size_v, el1002<imc>::product_code>;
template class el100x<imc, el1008<imc>::size_v, el1008<imc>::product_code>;

template class el2xxx<imc, el2004<imc>::size_v, el2004<imc>::product_code, el2004<imc>::name_v>;
template class el2xxx<imc, el2008<imc>::size_v, el2008<imc>::product_code, el2008<imc>::name_v>;
template class el2xxx<imc, el2809<imc>::size_v, el2809<imc>::product_code, el2809<imc>::name_v>;

}  // namespace tfc::ec::devices::beckhoff

// todo should we?
// #ifdef TFC_TESTING
#include <tfc/ipc/details/dbus_server_iface_mock.hpp>

namespace tfc::ec::devices::beckhoff {

// symbol export for ipc_manager_client_mock

using imc_m = tfc::ipc_ruler::ipc_manager_client_mock;

template class el100x<imc_m, el1002<imc_m>::size_v, el1002<imc_m>::product_code>;
template class el100x<imc_m, el1008<imc_m>::size_v, el1008<imc_m>::product_code>;

template class el2xxx<imc_m, el2004<imc_m>::size_v, el2004<imc_m>::product_code, el2004<imc_m>::name_v>;
template class el2xxx<imc_m, el2008<imc_m>::size_v, el2008<imc_m>::product_code, el2008<imc_m>::name_v>;
template class el2xxx<imc_m, el2809<imc_m>::size_v, el2809<imc_m>::product_code, el2809<imc_m>::name_v>;
}  // namespace tfc::ec::devices::beckhoff

// #endif
