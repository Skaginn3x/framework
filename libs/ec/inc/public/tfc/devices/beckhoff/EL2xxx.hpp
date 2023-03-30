#pragma once

namespace tfc::ec::devices::beckhoff {
template <size_t size, uint32_t pc>
class el200x : public base {
public:
  explicit el200x(boost::asio::io_context& ctx) : base(ctx) {}
  static constexpr auto product_code = pc;
  static constexpr auto vendor_id = 0x2;

  void process_data(uint8_t*, uint8_t* output) noexcept final {
    static_assert(size <= 8);

    *output = output_states_.to_ulong();
  }

  auto setup(ecx_contextt*, uint16_t slave_index) -> int final {
    // This can be constructed each time the slave goes from pre-op->op
    // Empty memory s.t. it does not grow the list.
    bool_receivers_.erase(bool_receivers_.begin(), bool_receivers_.end());
    for (size_t i = 0; i < size; i++) {
      bool_receivers_.emplace_back(
          tfc::ipc::bool_recv_cb::create(ctx_, fmt::format("EL200{}.{}.bool.out.{}", size, slave_index, i)));
      // TODO: Don't supply ipc signal name. IPC should do this by itself using confman?
      // As a test now, just connect it with the example signal that the test program creates
      bool_receivers_.back()->init(
          fmt::format("{}.{}.EL1008.5.in.{}", tfc::base::get_exe_name(), tfc::base::get_proc_name(), i),
          std::bind(&el200x::set_output, this, i, std::placeholders::_1));
    }
    return 1;
  }

private:
  auto set_output(size_t position, bool value) -> void { output_states_.set(position, value); }
  std::bitset<size> output_states_;
  std::vector<std::shared_ptr<tfc::ipc::bool_recv_cb>> bool_receivers_;
};

using el2008 = el200x<8, 0x7d83052>;
}  // namespace tfc::ec::devices::beckhoff