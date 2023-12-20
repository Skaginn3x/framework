#pragma once

#include <memory>

#include <fmt/core.h>

#include "abt/easycat.hpp"
#include "beckhoff/EK1xxx.hpp"
#include "beckhoff/EL1xxx.hpp"
#include "beckhoff/EL2xxx.hpp"
#include "beckhoff/EL3xxx.hpp"
#include "beckhoff/EL4xxx.hpp"
#include "beckhoff/EL9222.hpp"
#include "beckhoff/EQ2339.hpp"
#include "eilersen/4x60a.hpp"
#include "schneider/atv320.hpp"
#include "schneider/lxm32m.hpp"

namespace tfc::ec::devices {

template <typename...>
struct devices_type {};

// clang-format off
template <typename manager_client_t>
using devices_list = devices_type<
  abt::easyecat<manager_client_t>,
  beckhoff::ek1100,
  beckhoff::el1002<manager_client_t>,
  beckhoff::el1008<manager_client_t>,
  beckhoff::el1809<manager_client_t>,
  beckhoff::el2794<manager_client_t>,
  beckhoff::el2004<manager_client_t>,
  beckhoff::el2008<manager_client_t>,
  beckhoff::el2809<manager_client_t>,
  beckhoff::el9222,
  beckhoff::eq2339<manager_client_t>,
  //beckhoff::el3054,
  beckhoff::el4002,
  schneider::atv320::device<manager_client_t>,
  schneider::lxm32m<manager_client_t>,
  eilersen::e4x60a::e4x60a<manager_client_t>
>;
// clang-format on

// todo appendable type list
// https://b.atch.se/posts/constexpr-meta-container/
// https://mc-deltat.github.io/articles/stateful-metaprogramming-cpp20

template <typename device>
auto devices_equal(auto vendor_id, auto product_code) {
  return vendor_id == device::vendor_id && product_code == device::product_code;
}

template <typename manager_client_type, typename device_t>
auto get_impl(boost::asio::io_context& ctx,
              manager_client_type& client,
              uint16_t const slave_index,
              auto vendor_id,
              auto product_code,
              std::unique_ptr<base>& output) -> bool {
  if (devices_equal<device_t>(vendor_id, product_code)) {
    if constexpr (std::is_constructible_v<device_t, boost::asio::io_context&, manager_client_type&, uint16_t>) {
      output = std::make_unique<device_t>(ctx, client, slave_index);
    } else if constexpr (std::is_constructible_v<device_t, boost::asio::io_context&, uint16_t>) {
      output = std::make_unique<device_t>(ctx, slave_index);
    } else if constexpr (std::is_constructible_v<device_t, uint16_t>) {
      output = std::make_unique<device_t>(slave_index);
    } else {
      // clang-format off
      []<bool flag = false>() { static_assert(flag, "No matching constructor"); } ();
      // clang-format on
    }
    return true;
  }
  return false;
}

template <typename manager_client_type, typename... devices_t>
auto get_impl(boost::asio::io_context& ctx,
              manager_client_type& client,
              uint16_t const slave_index,
              auto vendor_id,
              auto product_code,
              devices_type<devices_t...>) -> std::unique_ptr<base> {
  std::unique_ptr<base> result{};
  (get_impl<manager_client_type, devices_t>(ctx, client, slave_index, vendor_id, product_code, result) || ...);
  if (result) {
    return result;
  }
  tfc::logger::logger log("missing_implementation");
  log.warn("No device found for slave: {}, vendor: {}, product_code: {} \n", slave_index, vendor_id, product_code);
  return std::make_unique<default_device>(slave_index);
}

template <typename manager_client_type>
auto get(boost::asio::io_context& ctx,
         manager_client_type& client,
         uint16_t const slave_index,
         auto vendor_id,
         auto product_code) -> std::unique_ptr<base> {
  return get_impl(ctx, client, slave_index, vendor_id, product_code, devices_list<manager_client_type>{});
}
}  // namespace tfc::ec::devices
