#pragma once

#include <bitset>
#include <cstdio>

#include "ethercat.h"
#include "tfc/devices/base.hpp"

class easyecat : public device_base {
public:
  static constexpr auto vendor_id = 0x79a;
  static constexpr auto product_code = 0xdefede;
  auto process_data(uint8_t* input, uint8_t* output) noexcept -> void final {
    // Print a line on change, later connect this to signals and do it properly
    bool chg = false;
    if (ana0_ != input[0] || ana1_ != input[1] || digital_input_ != input[6]){
      chg = true;
    }
    ana0_ = input[0];
    ana1_ = input[1];
    digital_input_ = input[6];
    *output = digital_output_;
    if (chg){
      printf("Ana0: 0x%x\tAna1: 0x%x\tDigital: %s\n", ana0_, ana1_, std::bitset<4>(digital_input_).to_string().c_str());
    }
  }
private:
  uint8_t digital_output_;
  uint8_t ana0_;
  uint8_t ana1_;
  uint8_t digital_input_;
};
