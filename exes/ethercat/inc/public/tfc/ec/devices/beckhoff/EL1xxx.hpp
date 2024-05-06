#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include <tfc/ec/devices/base.hpp>
#include <tfc/ipc/details/dbus_client_iface.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/ipc_fwd.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/utils/asio_fwd.hpp>

namespace tfc::ec::devices::beckhoff {

namespace asio = boost::asio;

template <typename manager_client_type,
          size_t size,
          std::array<std::size_t, size> entries,
          uint32_t pc,
          stx::basic_fixed_string name_v,
          template <typename description_t, typename manager_client_t> typename signal_t = ipc::signal>
class el1xxx final : public base<el1xxx<manager_client_type, size, entries, pc, name_v>> {
public:
  using input_pdo = std::array<std::uint8_t, (size / 9) + 1>;

  el1xxx(asio::io_context& ctx, manager_client_type& client, uint16_t const slave_index);
  static constexpr auto size_v = size;
  static constexpr auto entries_v = entries;
  static constexpr uint32_t product_code = pc;
  static constexpr uint32_t vendor_id = 0x2;
  static constexpr auto name = name_v;

  void pdo_cycle(input_pdo const& input, std::span<std::uint8_t>) noexcept;

  auto transmitters() const noexcept -> auto const& { return transmitters_; }

private:
  std::array<std::optional<bool>, size> last_values_{};
  using bool_signal_t = signal_t<ipc::details::type_bool, manager_client_type&>;
  std::array<std::shared_ptr<bool_signal_t>, size> transmitters_;
};

template <typename manager_client_type, template <typename, typename> typename signal_t = ipc::signal>
using el1002 = el1xxx<manager_client_type, 2, std::to_array<std::size_t>({ 1, 5 }), 0x3ea3052, "el1002", signal_t>;
template <typename manager_client_type, template <typename, typename> typename signal_t = ipc::signal>
using el1008 =
    el1xxx<manager_client_type, 8, std::to_array<std::size_t>({ 1, 5, 2, 6, 3, 7, 4, 8 }), 0x3f03052, "el1008", signal_t>;
template <typename manager_client_type, template <typename, typename> typename signal_t = ipc::signal>
using el1809 = el1xxx<manager_client_type,
                      16,
                      std::to_array<std::size_t>({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }),
                      0x7113052,
                      "el1809",
                      signal_t>;

using imc = tfc::ipc_ruler::ipc_manager_client;
extern template class el1xxx<imc, el1002<imc>::size_v, el1002<imc>::entries_v, el1002<imc>::product_code, el1002<imc>::name>;
extern template class el1xxx<imc, el1008<imc>::size_v, el1008<imc>::entries_v, el1008<imc>::product_code, el1008<imc>::name>;
extern template class el1xxx<imc, el1809<imc>::size_v, el1809<imc>::entries_v, el1809<imc>::product_code, el1809<imc>::name>;
}  // namespace tfc::ec::devices::beckhoff
