#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include <tfc/ec/devices/base.hpp>
#include <tfc/ipc/details/type_description.hpp>
#include <tfc/ipc_fwd.hpp>
#include <tfc/utils/asio_fwd.hpp>

namespace tfc::ec::devices::beckhoff {

namespace asio = boost::asio;

template <typename manager_client_type, size_t size, uint32_t pc>
class el100x final : public base {
public:
  el100x(asio::io_context& ctx, manager_client_type& client, uint16_t const slave_index);
  static constexpr auto size_v = size;
  static constexpr uint32_t product_code = pc;
  static constexpr uint32_t vendor_id = 0x2;

  void process_data(std::span<std::byte> input, std::span<std::byte>) noexcept final;

  auto transmitters() const noexcept -> auto const& {
    return transmitters_;
  }

private:
  std::array<bool, size> last_values_;
  using signal_t = ipc::signal<ipc::details::type_bool, manager_client_type>;
  std::vector<std::shared_ptr<signal_t>> transmitters_;
};

template <typename manager_client_type>
using el1002 = el100x<manager_client_type, 2, 0x3ea3052>;
template <typename manager_client_type>
using el1008 = el100x<manager_client_type, 8, 0x3f03052>;
}  // namespace tfc::ec::devices::beckhoff
