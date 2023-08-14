#pragma once

namespace tfc::ec::devices::beckhoff {
template <typename manager_client_type, size_t size, uint32_t pc, std::string_view const& name>
class el2xxx : public base {
public:
  explicit el2xxx(boost::asio::io_context& ctx, manager_client_type& client, uint16_t slave_index) : base(slave_index) {
    for (size_t i = 0; i < size; i++) {
      bool_receivers_.emplace_back(std::make_unique<tfc::ipc::bool_slot>(
          ctx, client, fmt::format("{}.{}.out.{}", name, slave_index, i), std::bind_front(&el2xxx::set_output, this, i)));
    }
  }

  static constexpr uint32_t product_code = pc;
  static constexpr uint32_t vendor_id = 0x2;

  void process_data(std::span<std::byte>, std::span<std::byte> output) noexcept final {
    static_assert(size == 8 || size == 4 || size == 16, "Invalid size");
    output[0] = static_cast<std::byte>(output_states_.to_ulong() & 0xff);
    if constexpr (size > 8) {
      output[1] = static_cast<std::byte>(output_states_.to_ulong() >> 8);
    }
  }

private:
  auto set_output(size_t position, bool value) -> void { output_states_.set(position, value); }

  std::bitset<size> output_states_;
  std::vector<std::unique_ptr<tfc::ipc::bool_slot>> bool_receivers_;
};

using std::string_view_literals::operator""sv;

constexpr auto el2008_name = "EL2008"sv;
template <typename manager_client_type>
using el2008 = el2xxx<manager_client_type, 8, 0x7d83052, el2008_name>;

constexpr auto el2004_name = "EL2004"sv;
template <typename manager_client_type>
using el2004 = el2xxx<manager_client_type, 4, 0x7d43052, el2004_name>;

constexpr auto el2809_name = "EL2809"sv;
template <typename manager_client_type>
using el2809 = el2xxx<manager_client_type, 16, 0xaf93052, el2809_name>;

// void process_data(std::span<std::byte>, std::span<std::byte> output) noexcept final {
//   output[0] = static_cast<std::byte>(output_states_.to_ulong());
//   output[1] = static_cast<std::byte>(output_states_.to_ulong() >> 8);
// }

}  // namespace tfc::ec::devices::beckhoff
