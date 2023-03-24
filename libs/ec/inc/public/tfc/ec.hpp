
#pragma once
#include <array>
#include <cassert>
#include <chrono>
#include <vector>
#include <boost/asio.hpp>

#include "devices/device.hpp"
#include "ecx.hpp"

namespace tfc::ec {
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

class context_t {
public:
  explicit context_t(std::string_view iface) : iface_(iface) {
    context_.userdata = static_cast<void*>(this);
    context_.port = &port_;
    context_.slavecount = &slave_count_;
    context_.slavelist = slavelist_.data();
    context_.maxslave = ecx::constants::max_slave;
    context_.maxgroup = ecx::constants::max_group;
    context_.grouplist = grouplist_.data();
    context_.esibuf = esibuf_.data();
    context_.esimap = esimap_.data();
    context_.elist = &elist_;
    context_.idxstack = &idxstack_;
    context_.ecaterror = &ecat_error_;
    context_.DCtime = &dc_time_;
    context_.SMcommtype = SMcommtype_.data();
    context_.PDOassign = PDOassign_.data();
    context_.PDOdesc = PDOdesc_.data();
    context_.eepSM = &eep_SM_;
    context_.eepFMMU = &eepFMMU_;
    context_.manualstatechange = 0;
    context_.esislave = 0;
    context_.FOEhook = nullptr;
    context_.EOEhook = nullptr;
  }
  context_t(const context_t&) = delete;
  auto operator=(const context_t&) -> context_t& = delete;

  ~context_t() {
    // Use slave 0 -> virtual for all
    // Set state to init
    slavelist_[0].state = EC_STATE_INIT;
    // Write the state
    ecx_writestate(&context_, 0);

    // Close the context
    ecx_close(&context_);
  }
  [[nodiscard]] auto init() -> bool { return ecx::init(&context_, iface_); }
  auto processdata(std::chrono::microseconds timeout) -> ecx::working_counter_t {

    auto retval = ecx::recieve_processdata(&context_, timeout);
    for(int i = 1; i < slave_count_+1; i++){
      slaves_[i]->process_data(slavelist_[i].inputs, slavelist_[i].outputs);
    }
    ecx_send_processdata(&context_);

    return retval;
  };
  /**
   * Scans the ethercat network and populates the slaves.
   * @param use_config_table
   * @return
   */
  [[nodiscard]] auto config_init(bool use_config_table) -> bool {
    if (!ecx::config_init(&context_, use_config_table)) {
      return false;
    }
    // Insert the base device into the vector.
    slaves_ = std::vector<std::unique_ptr<device_base>>();
    slaves_.reserve(slave_count_ + 1);
    slaves_.emplace_back(std::make_unique<default_device>());
    // Attach the callback to each slave
    for (int i = 1; i <= slave_count_; i++) {
      slaves_.emplace_back(get_device(slavelist_[i].eep_man, slavelist_[i].eep_id));
      slavelist_[i].PO2SOconfigx = slave_config_callback;
    }
    return true;
  };
  [[nodiscard]] auto iface() -> std::string_view { return iface_; }
  [[nodiscard]] auto slave_count() const -> size_t { return slave_count_; };
  [[nodiscard]] auto config_overlap_map_group(uint8_t group_index = 0) -> size_t {
    return ecx::config_overlap_map_group(&context_, std::span(io_.data(), io_.size()), group_index);
  }
  auto configdc() -> bool { return ecx::configdc(&context_); }
  auto statecheck(uint16_t slave_index,
                  ec_state requested_state,
                  std::chrono::microseconds timeout = ecx::constants::timeout_state) {
    return ecx::statecheck(&context_, slave_index, requested_state, timeout);
  }
  auto write_state(uint16_t slave_index, ec_state rqstState) -> ecx::working_counter_t {
    if (slave_index >= slave_count_) {
      return 0;
    }
    context_.slavelist[slave_index].state = rqstState;
    return ecx_writestate(&context_, slave_index);
  }
  auto slave_state(uint16_t slave_index) -> ec_state {
    if (slave_index >= slave_count_) {
      return EC_STATE_NONE;
    }
    return static_cast<ec_state>(slavelist_[slave_index].state);
  }
  /**
   * run the fieldbus evet cycle interval
   * @param ctx
   * @return
   */
  auto async_run (boost::asio::io_context& ctx, std::chrono::microseconds cycle) -> void{
    boost::asio::post([&](){
      auto timer = std::make_shared<boost::asio::steady_timer>(ctx);
      timer->expires_after(cycle);
      timer->async_wait([&, timer](std::error_code err){
        printf("HERE");
        if (err) {
          throw std::runtime_error(err.message());
        }
        printf("HERE");
        async_run(ctx, cycle);
      });
      processdata(ecx::constants::timeout_tx_to_rx);
    });
  }
private:
  /**
   * A callback function used to get passed the void* behaviour
   * of the underlying library. We only get a single void*
   * per context but not per slave. So this class instance
   * is embeded in the void* userdata. This function is a middle
   * function to relay the callbacks to the correct slave functions
   * with data.
   * @param context
   * @param slave_index
   * @return the number of slaves configured
   */
  static auto slave_config_callback(ecx_contextt* context, uint16_t slave_index) -> int {
    auto* instance = static_cast<context_t*>(context->userdata);
    ec_slavet& sl = context->slavelist[slave_index]; //NOLINT
    printf("product code: 0x%x\tvendor id: 0x%x\t slave index: %d name: %s, aliasaddr: %d\n", sl.eep_id,
           sl.eep_man, slave_index, sl.name, sl.aliasadr);
    instance->slaves_[slave_index]->setup(context, slave_index);
    return 1;
  }

  std::string iface_;
  ecx_contextt context_{};
  std::vector<std::unique_ptr<device_base>> slaves_;

  // Stack allocations for pointers inside ec_contextt.
  ecx_portt port_;
  std::array<ec_slavet, ecx::constants::max_slave> slavelist_;
  int slave_count_ = 0;
  std::array<ec_groupt, ecx::constants::max_group> grouplist_;
  std::array<uint8, ecx::constants::max_eeprom_buffer> esibuf_;
  std::array<uint32, ecx::constants::max_eeprom_bitmap> esimap_;
  ec_eringt elist_;
  ec_idxstackT idxstack_;
  uint8_t ecat_error_ = 0;
  int64_t dc_time_;
  std::array<ec_SMcommtypet, ecx::constants::max_concurrent_map_thread> SMcommtype_;
  std::array<ec_PDOassignt, ecx::constants::max_concurrent_map_thread> PDOassign_;
  std::array<ec_PDOdesct, ecx::constants::max_concurrent_map_thread> PDOdesc_;
  ec_eepromSMt eep_SM_;
  ec_eepromFMMUt eepFMMU_;

  // Timing related variables
  std::chrono::nanoseconds min_cycle_;
  std::chrono::nanoseconds max_cycle_;
  std::chrono::nanoseconds last_cycle_;
  std::chrono::nanoseconds cycle_sum_;
  size_t number_of_cycles_;
  std::array<std::byte, 4096> io_;
};
}  // namespace tfc::ec