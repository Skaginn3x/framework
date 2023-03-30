#pragma once

#include <boost/asio.hpp>

#include "tfc/ecx.hpp"

namespace tfc::ec::devices {
class base {
public:
  explicit base(boost::asio::io_context& ctx) : ctx_(ctx) {}
  static constexpr auto vendor_id = 0x0;
  static constexpr auto product_code = 0x0;
  virtual ~base() = default;
  // Default behaviour no data processing
  virtual void process_data(uint8_t*, uint8_t*) noexcept {};
  // Default behaviour, no setup
  virtual auto setup(ecx_contextt*, uint16_t) -> int { return 1; };

protected:
  boost::asio::io_context& ctx_;
};

class default_device : public base {
public:
  explicit default_device(boost::asio::io_context& ctx) : base(ctx) {}
  void process_data(uint8_t*, uint8_t*) noexcept override{};
  auto setup(ecx_contextt*, uint16_t) -> int override { return 1; };
};
}  // namespace tfc::ec::devices