#pragma once

namespace tfc::ec::devices::beckhoff {
template <typename manager_client_type, size_t size, uint32_t pc>
class el200x : public base {
public:
  explicit el200x(boost::asio::io_context& ctx, manager_client_type& client, uint16_t slave_index) : base(slave_index) {
    for (size_t i = 0; i < size; i++) {
      bool_receivers_.emplace_back(
          std::make_unique<tfc::ipc::bool_slot>(ctx, client, fmt::format("EL200{}.{}.out.{}", size, slave_index, i),
                                                std::bind_front(&el200x::set_output, this, i)));
    }
  }

  static constexpr uint32_t product_code = pc;
  static constexpr uint32_t vendor_id = 0x2;

  void process_data(std::span<std::byte>, std::span<std::byte> output) noexcept final {
    static_assert(size <= 8);
    //assert(output.size() == 1);
    output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0xff);
  }

private:
  auto set_output(size_t position, bool value) -> void { output_states_.set(position, value); }

  std::bitset<size> output_states_;
  std::vector<std::unique_ptr<tfc::ipc::bool_slot>> bool_receivers_;
};

template <typename manager_client_type>
using el2008 = el200x<manager_client_type, 8, 0x7d83052>;
template <typename manager_client_type>
using el2004 = el200x<manager_client_type, 4, 0x7d43052>;
}  // namespace tfc::ec::devices::beckhoff
