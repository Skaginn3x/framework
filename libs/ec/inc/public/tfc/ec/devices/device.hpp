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

using tfc::ec::devices::default_device;
using tfc::ec::devices::abt::easyecat;
using tfc::ec::devices::beckhoff::ek1100;
using tfc::ec::devices::beckhoff::el1008;
using tfc::ec::devices::beckhoff::el2008;
using tfc::ec::devices::beckhoff::el3054;
using tfc::ec::devices::beckhoff::el4002;
using tfc::ec::devices::schneider::atv320;

template <typename device>
auto devices_equal(auto vendor_id, auto product_code) {
  return vendor_id == device::vendor_id && product_code == device::product_code;
}

auto get(boost::asio::io_context& ctx, uint16_t const slave_index, auto vendor_id, auto product_code)
    -> std::unique_ptr<base> {
  if (devices_equal<atv320>(vendor_id, product_code))
    return std::make_unique<atv320>(ctx, slave_index);
  if (devices_equal<easyecat>(vendor_id, product_code))
    return std::make_unique<easyecat>(ctx, slave_index);
  if (devices_equal<ek1100>(vendor_id, product_code))
    return std::make_unique<ek1100>(slave_index);
  if (devices_equal<el1008>(vendor_id, product_code))
    return std::make_unique<el1008>(ctx, slave_index);
  if (devices_equal<el2008>(vendor_id, product_code))
    return std::make_unique<el2008>(ctx, slave_index);
  if (devices_equal<el3054>(vendor_id, product_code))
    return std::make_unique<el3054>(ctx, slave_index);
  if (devices_equal<el4002>(vendor_id, product_code))
    return std::make_unique<el4002>(ctx, slave_index);
  return std::make_unique<default_device>(slave_index);
}
}  // namespace tfc::ec::devices