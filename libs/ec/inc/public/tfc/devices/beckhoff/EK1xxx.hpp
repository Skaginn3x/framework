#pragma once

#include "tfc/devices/base.hpp"

class ek1100 : public device_base {
public:
  static constexpr auto product_code = 0x44c2c52;
  static constexpr auto vendor_id = 0x2;
};