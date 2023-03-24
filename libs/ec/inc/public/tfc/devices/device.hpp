#pragma once

#include <memory>

#include "abt/easycat.hpp"
#include "beckhoff/EK1xxx.hpp"
#include "beckhoff/EL1xxx.hpp"
#include "beckhoff/EL2xxx.hpp"
#include "beckhoff/EL3xxx.hpp"
#include "beckhoff/EL4xxx.hpp"
#include "schneider/atv320.hpp"

template <typename device>
auto devices_equal(auto vendor_id, auto product_code) {
  return vendor_id == device::vendor_id && product_code == device::product_code;
}

auto get_device(auto vendor_id, auto product_code) -> std::unique_ptr<device_base> {
  if (devices_equal<atv320>(vendor_id, product_code)) return std::make_unique<default_device>();
  if (devices_equal<easyecat>(vendor_id, product_code)) return std::make_unique<easyecat>();
  if (devices_equal<ek1100>(vendor_id, product_code)) return std::make_unique<ek1100>();
  if (devices_equal<el1008>(vendor_id, product_code)) return std::make_unique<el1008>();
  if (devices_equal<el2008>(vendor_id, product_code)) return std::make_unique<el2008>();
  if (devices_equal<el2008>(vendor_id, product_code)) return std::make_unique<el2008>();
  if (devices_equal<el3054>(vendor_id, product_code)) return std::make_unique<el3054>();
  if (devices_equal<el4002>(vendor_id, product_code)) return std::make_unique<el4002>();
  return std::make_unique<default_device>();
}
