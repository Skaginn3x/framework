#pragma once

#include "tfc/devices/base.hpp"

namespace tfc::ec::devices::beckhoff {
class ek1100 : public base {
public:
  explicit ek1100(boost::asio::io_context& ctx) : base(ctx) {}
  static constexpr auto product_code = 0x44c2c52;
  static constexpr auto vendor_id = 0x2;
};
}  // namespace tfc::ec::devices::beckhoff