#pragma once

template <size_t size, auto p_code>
class el400x : public device_base {
public:
  static constexpr auto product_code = p_code;
  static constexpr auto vendor_id = 0x2;

  void process_data(uint8_t*, uint8_t* output) noexcept final {
    if (output == nullptr)
      return;

    // if (std::chrono::high_resolution_clock::now() - point_ > std::chrono::microseconds (250)){
    for (size_t i = 0; i < size; i++) {
        value_[i] += 50;
    }
    point_ = std::chrono::high_resolution_clock::now();

    //}

    // Cast pointer type to uint16_t
    auto* out = reinterpret_cast<uint16_t*>(output);  // NOLINT
    for (size_t i = 0; i < size; i++) {
      out[i] = value_[i];
    }
  }

private:
  std::array<uint16_t, size> value_;
  std::chrono::time_point<std::chrono::high_resolution_clock> point_ = std::chrono::high_resolution_clock::now();
};

using el4002 = el400x<4, 0xfa23052>;
