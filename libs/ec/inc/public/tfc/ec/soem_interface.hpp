// attempt at c++ typesafe api for SOEM functions
#pragma once

// Drop V1 interface
#define EC_VER2

#include <chrono>
#include <memory>
#include <span>
#include <vector>

#include <tfc/utils/pragmas.hpp>
#include <tfc/stx/bitset_join.h>

// TODO: Contribute upstream a working clang build pipeline and
//  Try to advicate having these things cleaned up.

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wpacked)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wnewline-eof)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wnested-anon-types)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wgnu-anonymous-struct)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wdocumentation)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wreserved-macro-identifier)
// clang-format on
#include <ethercat.h>
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP

namespace ecx {

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

namespace constants {
static constexpr microseconds timeout_safe = microseconds(EC_TIMEOUTSAFE);
static constexpr microseconds timeout_rx_mailbox_cycle = microseconds(EC_TIMEOUTRXM);
static constexpr microseconds timeout_tx_to_rx = microseconds(EC_TIMEOUTRET);
static constexpr size_t max_slave = EC_MAXSLAVE;
static constexpr size_t max_group = EC_MAXGROUP;
static constexpr size_t max_eeprom_buffer = EC_MAXEEPBUF;
static constexpr size_t max_eeprom_bitmap = EC_MAXEEPBITMAP;
static constexpr size_t max_concurrent_map_thread = EC_MAX_MAPT;
}  // namespace constants

using index_t = std::pair<uint16_t, uint8_t>;
// Value returned is the working counter returned with this request.
// This means the amount of slaves that succesfully "did" this work
// It is almost impossible here to determine if that was the correct amount
// of slaves affected or not.
using working_counter_t = int32_t;

/// \brief Complete Access Support
/// If this value is true, support for uploading or downloading the complete CanOpen object is activated. The entire
/// data area belonging to the main index with all its subindices is read or written.
using complete_access_t = bool;

[[nodiscard]] inline auto sdo_write(ecx_contextt* context,
                                    uint16_t slave_index,
                                    index_t index,
                                    complete_access_t complete_access,
                                    std::ranges::view auto data,
                                    microseconds timeout) -> working_counter_t {
  return static_cast<working_counter_t>(ecx_SDOwrite(context, slave_index, index.first, index.second, complete_access,
                                                     static_cast<int>(data.size()), data.data(),
                                                     static_cast<int>(timeout.count())));
}
template <std::integral t>
inline auto sdo_write(ecx_contextt* context, uint16_t slave_index, index_t index, t value) -> working_counter_t {
  return sdo_write(context, slave_index, index, false, std::span(&value, sizeof(value)), constants::timeout_safe);
}

/// Sync channel 2: process data telegram protocol for incoming PDOs (master -> slave)
/// rx for slave, tx for master
template <uint8_t subindex = 0>
static constexpr index_t rx_pdo_assign = { 0x1C12, subindex }; // todo @omar where did you find rx pdo as 1c13
/// Sync channel 3: process data telegram protocol for outgoing PDOs (master <- slave)
/// tx for slave, rx for master
template <uint8_t subindex = 0>
static constexpr index_t tx_pdo_assign = { 0x1C13, subindex };

/// FYI
/// The parameterisation of the individual PDOs is set via the objects 0x1600 and 0x160x
/// (receive PDOs) and 0x1A00 and 0x1A0x (transmit PDOs). If device supports more mappings the `x` address applies.
/// Basically the setting of the sync channels and the configuration of the PDOs can only be
/// carried out in the "Pre−operational state.
/// rx for slave, tx for master
template <uint8_t subindex = 0>
static constexpr index_t rx_pdo_mapping = { 0x1600, subindex };  // todo @omar where did you find rx pdo as 1a00
/// tx for slave, rx for master
template <uint8_t subindex = 0>
static constexpr index_t tx_pdo_mapping = { 0x1A00, subindex };

template <typename value_t>
constexpr auto make_mapping_value() -> uint32_t {
  std::bitset<sizeof(value_t::index.first) * 8> index{ value_t::index.first };
  std::bitset<sizeof(value_t::index.second) * 8> sub_index{ value_t::index.second };
  std::bitset<8> constexpr some_constant_which_i_would_like_to_know_the_name_of{ sizeof(value_t) * 8 };
  auto result = tfc::stx::bitset_join(index, sub_index, some_constant_which_i_would_like_to_know_the_name_of);
  static_assert(result.size() == 32);
  return static_cast<uint32_t>(result.to_ulong());
}

[[nodiscard, maybe_unused]] inline auto recieve_processdata(ecx_contextt* context,
                                                            microseconds timeout = constants::timeout_tx_to_rx)
    -> working_counter_t {
  return static_cast<working_counter_t>(ecx_receive_processdata(context, static_cast<int>(timeout.count())));
}

/**
 *
 * @param context pointer to the soem context
 * @param iface a string name of the ethernet interface
 * @return false if failed
 * @warning This function only calls ecx_setupnic internally,
 * just call that function directly and get better control.
 * @todo Make this function return a custom std::error_cde
 */
[[nodiscard, maybe_unused]] inline auto init(ecx_contextt* context, std::string_view iface) -> bool {
  return ecx_init(context, iface.data()) > 0;
}
/**
 *  Setup the nic for soem communication, this is a wrapper around ecx_setupnic
 *  context->port
 * @param context
 * @param iface
 * @param secondary
 * @return
 */
[[nodiscard, maybe_unused]] inline auto setupnic(ecx_contextt* context, std::string_view iface, bool secondary) -> bool {
  return ecx_setupnic(context->port, iface.data(), secondary ? TRUE : FALSE) > 0;
}
/**
 * Scans the ethercat network and populates *slavelist
 * @param context pointer to the soem context
 * @param use_config_table use the config table
 * @return true if successful
 * @todo Make this function return a custom std::error_cde
 */
[[nodiscard, maybe_unused]] inline auto config_init(ecx_contextt* context, bool use_config_table) -> bool {
  return ecx_config_init(context, use_config_table ? 1 : 0) > 0;
}
/**
 * @param context
 * @param use_config_table
 * @return IOmap size
 */
inline auto config_map_group(ecx_contextt* context, std::ranges::view auto buffer, uint8_t group_index) -> size_t {
  return ecx_config_map_group(context, buffer.data(), group_index) > 0;
}

inline auto write_state(ecx_contextt* context, uint16_t slave_index) -> working_counter_t {
  return static_cast<ecx::working_counter_t>(ecx_writestate(context, slave_index));
}

inline auto config_map_group_aligned(ecx_contextt* context, std::ranges::view auto buffer, uint8_t group_index) -> size_t {
  return ecx_config_map_group_aligned(context, buffer.data(), group_index) > 0;
}

inline auto config_overlap_map_group(ecx_contextt* context, std::ranges::view auto buffer, uint8_t group_index) -> size_t {
  return ecx_config_overlap_map_group(context, buffer.data(), group_index) > 0;
}

[[maybe_unused]] inline auto configdc(ecx_contextt* context) -> bool {
  return ecx_configdc(context) == 1;
}

[[maybe_unused]] inline auto statecheck(ecx_contextt* context,
                                        uint16_t slave_index,
                                        ec_state requested_state,
                                        std::chrono::microseconds timeout) -> ec_state {
  return static_cast<ec_state>(ecx_statecheck(context, slave_index, requested_state, static_cast<int>(timeout.count())));
}

[[maybe_unused]] inline auto readstate(ecx_contextt* context) -> ec_state {
  return static_cast<ec_state>(ecx_readstate(context));
}

}  // namespace ecx
