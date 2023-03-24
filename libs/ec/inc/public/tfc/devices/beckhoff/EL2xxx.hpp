#pragma once

class el2008 : public device_base {
public:
  static constexpr auto product_code = 0x7d83052;
  static constexpr auto vendor_id = 0x2;

  void process_data(uint8_t*, uint8_t* output) noexcept final {
    //TODO connect to slots. For now just cycle
    if (std::chrono::high_resolution_clock::now() - point > std::chrono::milliseconds(100)){
      digital_output_ += 1;
      point = std::chrono::high_resolution_clock::now();
    }
    *output = digital_output_;
  }

private:
  uint8_t digital_output_;
  std::chrono::time_point<std::chrono::high_resolution_clock> point = std::chrono::high_resolution_clock::now();
};
