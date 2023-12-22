#pragma once

#include <array>
#include <boost/asio/io_context.hpp>
#include <cassert>
#include <chrono>
#include <vector>

#include <fmt/chrono.h>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ec/devices/device.hpp>
#include <tfc/ec/soem_interface.hpp>

namespace tfc::ec {
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

template <size_t pdo_buffer_size = 4096>
class context_t {
public:
  static constexpr std::string_view dbus_name{ "ethercat" };
  // There is support in SOEM and ethercat to split
  // your network into groups. There can even be
  // Many processing loops operating on the same
  // network at different frequencies and with
  // different slaves. This is neat but currently
  // Not beneficial to our use case.
  // This number indicates that "all" groups
  // are to be addressed when our code is
  // interacting with groups.

  explicit context_t(boost::asio::io_context& ctx, std::string_view iface)
      : ctx_(ctx), iface_(iface), logger_(fmt::format("Ethercat Context iface: ({})", iface)), client_(ctx_) {
    dbus_ = std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system());
    // dbus_->request_name(dbus::const_dbus_name<dbus_name>.data());
    dbus_->request_name("com.skaginn3x.ethercat");
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
    context_.manualstatechange = 1;  // Internal SOEM code changes ethercat states if not set.

    if (!ecx::init(&context_, iface_)) {
      // TODO: swith for error_code
      throw std::runtime_error("Failed to init, no socket connection");
    }
  }

  context_t(const context_t&) = delete;

  auto operator=(const context_t&) -> context_t& = delete;

  ~context_t() {
    running_ = false;
    if (check_thread_ != nullptr) {
      check_thread_->join();
    }
    // Use slave 0 -> virtual for all
    // Set state to init
    slavelist_[0].state = EC_STATE_INIT;
    // Write the state
    ecx_writestate(&context_, 0);

    // Close the context
    ecx_close(&context_);
  }

  auto processdata(std::chrono::microseconds timeout) -> ecx::working_counter_t {
    ecx_send_overlap_processdata(&context_);
    auto wkc = ecx::recieve_processdata(&context_, timeout);
    std::span<std::byte> input;
    std::span<std::byte> output;
    for (size_t i = 1; i < slave_count() + 1; i++) {
      if (slavelist_[i].inputs != nullptr) {
        input = { reinterpret_cast<std::byte*>(slavelist_[i].inputs), static_cast<size_t>(slavelist_[i].Ibytes) };
      }
      if (slavelist_[i].outputs != nullptr) {
        output = { reinterpret_cast<std::byte*>(slavelist_[i].outputs), static_cast<size_t>(slavelist_[i].Obytes) };
      }
      slaves_[i]->process_data(input, output);
    }

    return wkc;
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
      slaves_.emplace_back(
          devices::get(dbus_, client_, static_cast<uint16_t>(i), slavelist_[i].eep_man, slavelist_[i].eep_id));
      slaves_.back()->set_sdo_write_cb([this, i](ecx::index_t idx, ecx::complete_access_t acc, std::span<std::byte> data,
                                                 std::chrono::microseconds microsec) -> ecx::working_counter_t {
        return ecx::sdo_write(&context_, static_cast<uint16_t>(i), idx, acc, data, microsec);
      });
      slavelist_[i].PO2SOconfigx = slave_config_callback;
    }
    slave_list_as_span_with_master()[0].state = EC_STATE_PRE_OP | EC_STATE_ACK;
    ecx_writestate(&context_, 0);
    auto lowest = ecx::statecheck(&context_, 0, EC_STATE_PRE_OP, milliseconds(100));
    return lowest == EC_STATE_PRE_OP || lowest == (EC_STATE_ACK | EC_STATE_PRE_OP);
  }

  [[nodiscard]] auto iface() -> std::string_view { return iface_; }

  [[nodiscard]] auto slave_count() const -> size_t { return static_cast<size_t>(slave_count_); }

  auto configdc() -> bool { return ecx::configdc(&context_); }

  auto statecheck(uint16_t slave_index,
                  ec_state requested_state,
                  std::chrono::microseconds timeout = ecx::constants::timeout_safe) {
    return ecx::statecheck(&context_, slave_index, requested_state, timeout);
  }

  /**
   * Start the async cycle of monitoring the fieldbus
   * and processing IO's
   */
  auto async_start() -> std::error_code {
    if (!config_init(false)) {
      // TODO: Switch for error_code
      throw std::runtime_error("No slaves found!");
    }
    ecx::config_overlap_map_group(&context_, std::span(io_.data(), io_.size()), 0);

    if (!configdc()) {
      throw std::runtime_error("Failed to configure dc");
    }

    slave_list_as_span_with_master()[0].state = EC_STATE_SAFE_OP;
    ecx::write_state(&context_, 0);
    auto start = high_resolution_clock::now();
    auto found_state = statecheck(0, EC_STATE_SAFE_OP, milliseconds(2000));
    if (found_state != EC_STATE_SAFE_OP) {
      logger_.warn("Found State {} in {} expected {}", static_cast<int>(found_state),
                   duration_cast<milliseconds>(high_resolution_clock::now() - start), static_cast<int>(EC_STATE_SAFE_OP));
    }

    auto value = processdata(milliseconds{ 100 });
    if (value == EC_NOFRAME) {
      logger_.warn("No frame received inital pdo");
    }

    context_.slavelist[0].state = EC_STATE_OPERATIONAL;
    ecx::write_state(&context_, 0);

    processdata(milliseconds{ 2000 });
    // Start async loop
    expected_wkc_ = context_.grouplist->outputsWKC * 2 + context_.grouplist->inputsWKC;
    // Start in ok
    wkc_ = expected_wkc_;
    async_wait(true);
    check_thread_ = std::make_unique<std::thread>(&context_t::check_state, this, 0);
    return {};
  }

private:
  auto async_wait(bool first_iteration = false) -> void {
    auto timer = std::make_shared<boost::asio::steady_timer>(ctx_);
    if (first_iteration) {
      timer->expires_after(std::chrono::microseconds(0));
    } else {
      auto sleep_time = milliseconds(1) - (std::chrono::high_resolution_clock::now() - cycle_start_);
      timer->expires_after(sleep_time);
    }
    cycle_start_with_sleep_ = std::chrono::high_resolution_clock::now();
    timer->async_wait([this, timer](auto&& PH1) { fieldbus_roundtrip(std::forward<decltype(PH1)>(PH1)); });
  }

  auto fieldbus_roundtrip(std::error_code err) -> void {
    cycle_start_ = std::chrono::high_resolution_clock::now();
    if (err) {
      return;
    }
    int32_t last_wkc = wkc_;
    wkc_ = processdata(microseconds{ 100 });
    if (wkc_ < expected_wkc_ && wkc_ != last_wkc) {  // Don't wot over an already logged fault.
      logger_.warn("Working counter got {} expected {}", wkc_, expected_wkc_);
    }
    while (ecx_iserror(&context_) != 0U) {
      logger_.error("Ethercat context error: {}", ecx_elist2string(&context_));
    }
    // Update counter and timers now that this cycle is complete
    last_cycle_with_sleep_ = std::chrono::high_resolution_clock::now() - cycle_start_with_sleep_;
    min_cycle_with_sleep_ = std::min(min_cycle_with_sleep_, last_cycle_with_sleep_);
    max_cycle_with_sleep_ = std::max(max_cycle_with_sleep_, last_cycle_with_sleep_);

    last_cycle_ = std::chrono::high_resolution_clock::now() - cycle_start_;
    min_cycle_ = std::min(min_cycle_, last_cycle_);
    max_cycle_ = std::max(max_cycle_, last_cycle_);

    if (cycle_count_ % 10'000 == 0 and false) {
      // log the max cycle time
      logger_.trace("Ethercat max cycle time: {} us", max_cycle_.count());
      logger_.trace("Ethercat max cycle time with sleep: {} us", max_cycle_with_sleep_.count());
      logger_.trace("Ethercat min cycle time: {} us", min_cycle_.count());
      logger_.trace("Ethercat min cycle time with sleep: {} us", min_cycle_with_sleep_.count());
    }

    cycle_count_++;

    if (last_cycle_with_sleep_ > std::chrono::milliseconds(100)) {
      logger_.warn("Ethercat cycle time is too long: {}",
                   std::chrono::duration_cast<std::chrono::microseconds>(last_cycle_with_sleep_));
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
  auto check_state(uint8_t group_index = 0) -> void {
    auto& grp = group_list_as_span()[group_index];
    while (running_) {
      if (grp.docheckstate == 1 || wkc_ < expected_wkc_) {
        grp.docheckstate = FALSE;
        ecx_readstate(&context_);
        auto slaves = slave_list_as_span();
        uint16_t slave_index = 1;
        for (ec_slave& slave : slaves) {
          if (slave.state != EC_STATE_OPERATIONAL) {
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
          }
          if (slave.islost == 1) {
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
      std::this_thread::sleep_for(milliseconds(10));
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
    self->logger_.trace(
        "Setting up\nproduct code: {:#x}\nvendor id: {:#x}\nslave index: {}\nname: {}\naliasaddr: {}\nhasDC: {}\nstate : "
        "{}\nSupportes CoE Complete access: {}\n",
        sl.eep_id, sl.eep_man, slave_index, sl.name, sl.aliasadr, sl.hasdc, sl.state,
        (sl.CoEdetails & ECT_COEDET_SDOCA) != 0);
    return self->slaves_[slave_index]->setup();
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
  bool running_ = true;

  tfc::ipc_ruler::ipc_manager_client client_;

  // Timing related variables
  std::chrono::nanoseconds min_cycle_with_sleep_ = std::chrono::nanoseconds::max();
  std::chrono::nanoseconds max_cycle_with_sleep_ = std::chrono::nanoseconds::min();
  std::chrono::nanoseconds last_cycle_with_sleep_ = std::chrono::nanoseconds::zero();
  std::chrono::nanoseconds last_cycle_ = std::chrono::nanoseconds::zero();
  std::chrono::nanoseconds min_cycle_ = std::chrono::nanoseconds::max();
  std::chrono::nanoseconds max_cycle_ = std::chrono::nanoseconds::min();
  std::chrono::time_point<std::chrono::high_resolution_clock> cycle_start_with_sleep_;
  std::chrono::time_point<std::chrono::high_resolution_clock> cycle_start_;
  size_t cycle_count_ = 0;
  int32_t expected_wkc_ = 0;
  int32_t wkc_ = 0;
  std::array<std::byte, pdo_buffer_size> io_;
  std::unique_ptr<std::thread> check_thread_;
  std::shared_ptr<sdbusplus::asio::connection> dbus_;
};

// Template deduction guide
template <size_t pdo_buffer_size = 4096>
context_t(boost::asio::io_context&, std::string_view) -> context_t<pdo_buffer_size>;
}  // namespace tfc::ec
