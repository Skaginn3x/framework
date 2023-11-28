#pragma once

#include <cstddef>
#include <cstdint>

#include <boost/asio/io_context.hpp>

#include "tfc/ec/devices/base.hpp"


namespace tfc::ec::devices::eilersen::e4x60 {

namespace asio = boost::asio;

struct pdo_input {

};
static_assert(sizeof(pdo_input) == 18);

struct pdo_output {

};
static_assert(sizeof(pdo_input) == 4);


template <typename manager_client_type,
          template <typename description_t, typename manager_client_t> typename signal_t = ipc::signal>
class e4x60a final : public base {
public:
  e4x60a(asio::io_context& ctx, manager_client_type& client, std::uint16_t slave_index) : base{ slave_index } {}
  void process_data(std::span<std::byte> in, std::span<std::byte> out) final {
    printf("hello world\n");
  }
  static constexpr uint32_t product_code = 0x1040;
  static constexpr uint32_t vendor_id = 0x726;
};
}  // namespace tfc::ec::devices::eilersen::e4x60

