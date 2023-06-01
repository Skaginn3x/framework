#pragma once

#include <memory>

#include "abt/easycat.hpp"
#include "beckhoff/EK1xxx.hpp"
#include "beckhoff/EL1xxx.hpp"
#include "beckhoff/EL2xxx.hpp"
#include "beckhoff/EL3xxx.hpp"
#include "beckhoff/EL4xxx.hpp"
#include "schneider/atv320.hpp"

namespace tfc::ec::devices {

using abt::easyecat;
using beckhoff::ek1100;
using beckhoff::el1008;
using beckhoff::el2004;
using beckhoff::el2008;
using beckhoff::el3054;
using beckhoff::el4002;
using schneider::atv320;

template <typename device>
auto devices_equal(auto vendor_id, auto product_code) {
  return vendor_id == device::vendor_id && product_code == device::product_code;
}

template <typename manager_client_type>
auto get(boost::asio::io_context& ctx,
         manager_client_type& client,
         uint16_t const slave_index,
         auto vendor_id,
         auto product_code) -> std::unique_ptr<base> {
  if (devices_equal<atv320<manager_client_type>>(vendor_id, product_code))
    return std::make_unique<atv320<manager_client_type>>(ctx, client, slave_index);
  if (devices_equal<easyecat<manager_client_type>>(vendor_id, product_code))
    return std::make_unique<easyecat<manager_client_type>>(ctx, client, slave_index);
  if (devices_equal<ek1100>(vendor_id, product_code))
    return std::make_unique<ek1100>(slave_index);
  if (devices_equal<el1008<manager_client_type>>(vendor_id, product_code))
    return std::make_unique<el1008<manager_client_type>>(ctx, client, slave_index);
  if (devices_equal<el2004<manager_client_type>>(vendor_id, product_code))
    return std::make_unique<el2004<manager_client_type>>(ctx, client, slave_index);
  if (devices_equal<el2008<manager_client_type>>(vendor_id, product_code))
    return std::make_unique<el2008<manager_client_type>>(ctx, client, slave_index);
  if (devices_equal<el3054>(vendor_id, product_code))
    return std::make_unique<el3054>(ctx, slave_index);
  if (devices_equal<el4002>(vendor_id, product_code))
    return std::make_unique<el4002>(ctx, slave_index);

  return std::make_unique<default_device>(slave_index);
  throw std::runtime_error(fmt::format("Device not supported vendor_id: 0x{0:x}, product_code: 0x{1:x}", vendor_id, product_code));
}
}  // namespace tfc::ec::devices
