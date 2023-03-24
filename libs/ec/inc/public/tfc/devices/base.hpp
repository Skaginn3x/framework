#pragma once

class device_base {
public:
  static constexpr auto vendor_id = 0x0;
  static constexpr auto product_code = 0x0;
  virtual ~device_base() = default;
  // Default behaviour no data processing
  virtual void process_data(uint8_t*, uint8_t*) noexcept {};
  // Default behaviour, no setup
  virtual auto setup(ecx_contextt*, uint16_t) -> int { return 1; };
};

class default_device : public device_base {
  void process_data(uint8_t*, uint8_t* ) noexcept override{};
  auto setup(ecx_contextt*, uint16_t) -> int override { return 1; };
};
