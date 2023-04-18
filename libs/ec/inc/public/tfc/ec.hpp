#pragma once

#include <array>
#include <boost/asio/io_context.hpp>
#include <cassert>
#include <chrono>
#include <vector>

#include "ec/devices/device.hpp"
#include "ec/soem_interface.hpp"

namespace tfc::ec {
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

template <size_t pdo_buffer_size = 4096>
class context_t {
public:
  // There is support in SOEM and ethercat to split
  // your network into groups. There can even be
  // Many processing loops operating on the same
  // network at different frequencies and with
  // different slaves. This is neat but currently
  // Not beneficial to our use case.
  // This number indicates that "all" groups
  // are to be addressed when our code is
  // interacting with groups.
  static constexpr uint8_t all_groups = 0;
  static constexpr uint16_t all_slaves = 0;
  explicit context_t(boost::asio::io_context& ctx, std::string_view iface)
      : ctx_(ctx), iface_(iface), logger_(fmt::format("Ethercat Context iface: ({})", iface)) {
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
    for (size_t i = 1; i < slave_count() + 1; i++) {
      std::span<std::byte> input;
      std::span<std::byte> output;
      if (slavelist_[i].inputs != nullptr) {
        input = { reinterpret_cast<std::byte*>(slavelist_[i].inputs), static_cast<size_t>(slavelist_[i].Ibytes) };
      }
      if (slavelist_[i].outputs != nullptr) {
        output = { reinterpret_cast<std::byte*>(slavelist_[i].outputs), static_cast<size_t>(slavelist_[i].Obytes) };
      }
      slaves_[i]->process_data(input, output);
    }
    ecx_send_processdata(&context_);

    return retval;
  }
  /**
   * Scans the ethercat network and populates the slaves.
   * @param use_config_table bool whether to use config table or not
   * @return returns true for success
   */
  [[nodiscard]] auto config_init(bool use_config_table) -> bool {
    if (!ecx::config_init(&context_, use_config_table)) {
      return false;
    }
    // Insert the base device into the vector.
    slaves_ = std::vector<std::unique_ptr<devices::base>>();
    slaves_.reserve(slave_count() + 1);
    slaves_.emplace_back(std::make_unique<devices::default_device>(0));
    // Attach the callback to each slave
    for (size_t i = 1; i <= slave_count(); i++) {
      slaves_.emplace_back(devices::get(ctx_, static_cast<uint16_t>(i), slavelist_[i].eep_man, slavelist_[i].eep_id));
      slavelist_[i].PO2SOconfigx = slave_config_callback;
    }
    return true;
  }
  [[nodiscard]] auto iface() -> std::string_view { return iface_; }
  [[nodiscard]] auto slave_count() const -> size_t { return static_cast<size_t>(slave_count_); }
  auto config_map_group(uint8_t group_index = all_groups) -> size_t {
    return ecx::config_map_group(&context_, std::span(io_.data(), io_.size()), group_index);
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
    slave_list_as_span_with_master()[slave_index].state = rqstState;
    return static_cast<ecx::working_counter_t>(ecx_writestate(&context_, slave_index));
  }
  auto slave_state(uint16_t slave_index) -> ec_state {
    if (slave_index >= slave_count_) {
      return EC_STATE_NONE;
    }
    return static_cast<ec_state>(slavelist_[slave_index].state);
  }

  /**
   * Start the async cycle of monitoring the fieldbus
   * and processing IO's
   */
  auto async_start() -> std::error_code {
    if (!init()) {
      // TODO: swith for error_code
      throw std::runtime_error("Failed to init, no socket connection");
    }
    if (!config_init(false)) {
      // TODO: Switch for error_code
      throw std::runtime_error("No slaves found!");
    }

    config_map_group();

    if (!configdc()) {
      throw std::runtime_error("Failed to configure dc");
    }

    statecheck(all_slaves, EC_STATE_SAFE_OP, ecx::constants::timeout_state * 4);

    // Do a single read/write on the slaves to activate their outputs.
    processdata(ecx::constants::timeout_tx_to_rx);

    write_state(all_slaves, EC_STATE_OPERATIONAL);

    for (int i = 0; i < 10; i++) {
      processdata(ecx::constants::timeout_tx_to_rx);
      if (slave_state(all_slaves) == EC_STATE_OPERATIONAL) {
        // Start async loop
        async_wait();
        return {};
      }
    }

    std::string slave_status;
    ecx_readstate(&context_);
    auto slave_list = slave_list_as_span_with_master();
    for (auto& slave : slave_list) {
      if (slave.state != EC_STATE_OPERATIONAL) {
        slave_status += fmt::format("slave -> {} is 0x{} (AL-status=0x{} {})\n", slave.name, slave.state, slave.ALstatuscode,
                                    ec_ALstatuscode2string(slave.ALstatuscode));
      }
    }
    throw std::runtime_error(slave_status);
  }

private:
  auto async_wait() -> void {
    auto timer = std::make_shared<boost::asio::steady_timer>(ctx_);
    timer->expires_after(std::chrono::microseconds(1000));
    timer->async_wait([this, timer](auto&& PH1) { fieldbus_roundtrip(std::forward<decltype(PH1)>(PH1)); });
  }
  auto fieldbus_roundtrip(std::error_code err) -> void {
    if (err) {
      return;
    }
    size_t const expected_wkc = context_.grouplist->outputsWKC * 2 + context_.grouplist->inputsWKC;
    if (processdata(ecx::constants::timeout_tx_to_rx) < expected_wkc) {
      check_state();
    }
    while (ecx_iserror(&context_) != 0U) {
      logger_.error("Ethercat context error: {}", ecx_elist2string(&context_));
    }
    async_wait();
  }
  /**
   * Check the state of attached slaves.
   * If the slaves are no longer in operational mode. Attempt to
   * switch them to operational mode. And if the slaves have
   * been lost attempt to add them again.
   * @param group_index 0 for all groups
   */
  auto check_state(uint8_t group_index = all_groups) -> void {
    auto& grp = group_list_as_span()[group_index];
    grp.docheckstate = FALSE;
    ecx_readstate(&context_);
    auto slaves = slave_list_as_span();
    uint16_t slave_index = 1;
    for (ec_slave& slave : slaves) {
      if (slave.group != group_index) {
        /* This slave is part of another group: do nothing */
      } else if (slave.state != EC_STATE_OPERATIONAL) {
        grp.docheckstate = TRUE;
        if (slave.state == EC_STATE_SAFE_OP + EC_STATE_ERROR) {
          logger_.warn("Slave {}, {} is in SAFE_OP+ERROR, attempting ACK", slave_index, slave.name);
          slave.state = EC_STATE_SAFE_OP + EC_STATE_ACK;
          ecx_writestate(&context_, slave_index);
        } else if (slave.state == EC_STATE_SAFE_OP) {
          logger_.warn("Slave {}, {} is in SAFE_OP, change to OPERATIONAL", slave_index, slave.name);
          slave.state = EC_STATE_OPERATIONAL;
          ecx_writestate(&context_, slave_index);
        } else if (slave.state > EC_STATE_NONE) {
          if (ecx_reconfig_slave(&context_, slave_index, EC_TIMEOUTRET) != 0) {
            slave.islost = FALSE;
            logger_.warn("Slave {}, {} reconfigured", slave_index, slave.name);
          }
        } else if (slave.islost == 0) {
          ecx_statecheck(&context_, slave_index, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
          if (slave.state == EC_STATE_NONE) {
            slave.islost = TRUE;
            logger_.warn("Slave {}, {} lost", slave_index, slave.name);
          }
        }
      } else if (slave.islost != 0) {
        if (slave.state != EC_STATE_NONE) {
          slave.islost = FALSE;
          logger_.info("Slave {}, {} found", slave_index, slave.name);
        } else if (ecx_recover_slave(&context_, slave_index, EC_TIMEOUTRET) != 0) {
          slave.islost = FALSE;
          logger_.info("Slave {}, {} recovered", slave_index, slave.name);
        }
      }
      slave_index++;
    }

    if (context_.grouplist->docheckstate == 0) {
      logger_.info("All slaves resumed OPERATIONAL");
    }
  }
  /**
   * A callback function used to get passed the void* behaviour
   * of the underlying library. We only get a single void*
   * per context but not per slave. So this class instance
   * is embeded in the void* userdata. This function is a middle
   * function to relay the callbacks to the correct slave functions
   * with data.
   * @param context pointer to soem context
   * @param slave_index index of slave
   * @return the number of slaves configured
   */
  static auto slave_config_callback(ecx_contextt* context, uint16_t slave_index) -> int {
    auto* self = static_cast<context_t*>(context->userdata);
    ec_slavet& sl = self->slave_list_as_span_with_master()[slave_index];
    self->logger_.trace("Setting up\nproduct code: {:#x}\nvendor id: {:#x}\nslave index: {}\nname: {}\naliasaddr: {}",
                        sl.eep_id, sl.eep_man, slave_index, sl.name, sl.aliasadr);
    self->slaves_[slave_index]->setup(context, slave_index);
    return 1;
  }

  [[nodiscard]] auto slave_list_as_span() -> std::span<ec_slave> {
    // Slave at index 0 is reserved for actions on all slaves.
    return { &std::span<ec_slave>(context_.slavelist, slave_count() + 1)[1], slave_count() };
  }

  [[nodiscard]] auto slave_list_as_span_with_master() -> std::span<ec_slave> {
    return { context_.slavelist, slave_count() + 1 };
  }

  [[nodiscard]] auto group_list_as_span() -> std::span<ec_group> { return { context_.grouplist, 1 }; }

  boost::asio::io_context& ctx_;
  std::string iface_;
  tfc::logger::logger logger_;
  ecx_contextt context_{};
  std::vector<std::unique_ptr<devices::base>> slaves_;

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
  std::array<std::byte, pdo_buffer_size> io_;
};

// Template deduction guide
template <size_t pdo_buffer_size = 4096>
context_t(boost::asio::io_context&, std::string_view) -> context_t<pdo_buffer_size>;
}  // namespace tfc::ec
