#pragma once

#include <memory>

#include "abt/easycat.hpp"
#include "beckhoff/EK1xxx.hpp"
#include "beckhoff/EL1xxx.hpp"
#include "beckhoff/EL2xxx.hpp"
#include "beckhoff/EL3xxx.hpp"
#include "beckhoff/EL4xxx.hpp"
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
  beckhoff::el1008<manager_client_t>,
  beckhoff::el2004<manager_client_t>,
  beckhoff::el2008<manager_client_t>,
  beckhoff::el3054,
  beckhoff::el4002,
  schneider::atv320<manager_client_t>,
  schneider::lxm32m<manager_client_t>
>;
// clang-format on

using abt::easyecat;
using beckhoff::ek1100;
using beckhoff::el1008;
using beckhoff::el2008;
using beckhoff::el3054;
using beckhoff::el4002;
using schneider::atv320;
using schneider::lxm32m;

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
  return result;
}

template <typename manager_client_type>
auto get(boost::asio::io_context& ctx,
         manager_client_type& client,
         uint16_t const slave_index,
         auto vendor_id,
         auto product_code) -> std::unique_ptr<base> {
  return get_impl(ctx, client, slave_index, vendor_id, product_code, devices_list<manager_client_type>{});
  //
  //  if (devices_equal<atv320<manager_client_type>>(vendor_id, product_code))
  //    return std::make_unique<atv320<manager_client_type>>(ctx, client, slave_index);
  //  if (devices_equal<lxm32m<manager_client_type>>(vendor_id, product_code))
  //    return std::make_unique<lxm32m<manager_client_type>>(ctx, client, slave_index);
  //  if (devices_equal<easyecat<manager_client_type>>(vendor_id, product_code))
  //    return std::make_unique<easyecat<manager_client_type>>(ctx, client, slave_index);
  //  if (devices_equal<ek1100>(vendor_id, product_code))
  //    return std::make_unique<ek1100>(slave_index);
  //  if (devices_equal<el1008<manager_client_type>>(vendor_id, product_code))
  //    return std::make_unique<el1008<manager_client_type>>(ctx, client, slave_index);
  //  if (devices_equal<el2008<manager_client_type>>(vendor_id, product_code))
  //    return std::make_unique<el2008<manager_client_type>>(ctx, client, slave_index);
  //  if (devices_equal<el3054>(vendor_id, product_code))
  //    return std::make_unique<el3054>(ctx, slave_index);
  //  if (devices_equal<el4002>(vendor_id, product_code))
  //    return std::make_unique<el4002>(ctx, slave_index);
  //  return std::make_unique<default_device>(slave_index);
}
}  // namespace tfc::ec::devices
