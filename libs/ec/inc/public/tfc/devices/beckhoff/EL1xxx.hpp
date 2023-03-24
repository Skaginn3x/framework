#pragma once

class el1008 : public device_base {
public:
  static constexpr auto product_code = 0x3f03052;
  static constexpr auto vendor_id = 0x2;

  void process_data(uint8_t* input, uint8_t*) noexcept final {
    if (input == nullptr)
      return;
    bool chg = false;
    if (digital_input_ != input[0]) {
      chg = true;
    }
    digital_input_ = input[0];

    if (chg) {
      printf("EL1100, input: %s\n", std::bitset<8>(digital_input_).to_string().c_str());
    }
  }

private:
  uint8_t digital_input_;
};
