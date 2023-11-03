#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string_view>
#include <system_error>
#include <vector>

#include <fmt/format.h>
#include <boost/asio.hpp>
#include <tfc/ec/devices/base.hpp>
#include <tfc/ipc.hpp>

namespace tfc::ec::devices::beckhoff {

namespace asio = boost::asio;

template <typename manager_client_type, template <typename, typename> typename signal_t = ipc::signal>
class eq2339 final : public base {
public:
  static constexpr size_t size = 16;
  static constexpr std::string_view name{ "EQ2339_0022" };
  static constexpr auto product_code = 0x9234452;
  static constexpr uint32_t vendor_id = 0x2;

  eq2339(asio::io_context& ctx, manager_client_type& client, uint16_t slave_index) : base(slave_index) {
    for (size_t i = 0; i < size; i++) {
      receivers_.emplace_back(std::make_shared<tfc::ipc::slot<ipc::details::type_bool, manager_client_type&>>(
          ctx, client, fmt::format("{}.slave{}.out{}", name, slave_index, i), "Digital output",
          std::bind_front(&eq2339::set_output, this, i)));

      transmitters_.emplace_back(std::make_shared<signal_t<ipc::details::type_bool, manager_client_type&>>(
          ctx, client, fmt::format("{}.slave{}.in{}", name, slave_index, i), "Digital input"));
    }
  }

  void process_data(std::span<std::byte> input, std::span<std::byte> output) noexcept override {
    for (size_t byte = 0; byte <= 1; byte++) {
      for (size_t bits = 0; bits < 8 && size - ((byte * 8) + bits) > 0; bits++) {
        auto const value = static_cast<bool>(static_cast<uint8_t>(input[byte]) & (1 << bits));
        const size_t bit_index = byte * 8 + bits;
        if (value != last_values_[bit_index]) {
          transmitters_[bit_index]->async_send(value, [this](std::error_code error, size_t) {
            if (error) {
              logger_.error("Ethercat {}, error transmitting : {}", name, error.message());
            }
          });
        }
        last_values_[bit_index] = value;
      }
    }

    output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0xff);
    output[1] = static_cast<std::byte>(output_states_.to_ulong() >> 8);
  }

  auto set_output(size_t position, bool value) -> void { output_states_.set(position, value); }

  auto setup() -> int final { return 1; }

  auto transmitters() const noexcept -> auto const& { return transmitters_; }

private:
  std::bitset<size> output_states_;
  std::array<bool, size> last_values_{};
  std::vector<std::shared_ptr<ipc::slot<ipc::details::type_bool, manager_client_type&>>> receivers_;
  std::vector<std::shared_ptr<signal_t<ipc::details::type_bool, manager_client_type&>>> transmitters_;
};
}  // namespace tfc::ec::devices::beckhoff
