#pragma once

#include "tfc/ecx.hpp"
auto debug_print(const std::string& name, bool state) -> std::string {
  return name + (state ? ": true" : ": false");
}

template <size_t size, auto p_code>
class el305x : public device_base {
public:
  static constexpr auto product_code = p_code;
  static constexpr auto vendor_id = 0x2;

  int setup(ecx_contextt * context, uint16_t slave) override {
    // Set siemens bits true
    // and enable compact mode
    // Each input settings field is in 0x8000 + offset * 0x10
    // So 0 -> 0x80000
    // 1 -> 0x8010
    // 2 -> 0x8020
    // This depends on size
    for(size_t i = 0; i < size; i++){
      uint16_t const settings_index = 0x8000 + (i * 0x10);
      auto wkc = ecx::sdo_write<bool>(context, slave, { settings_index, 0x05 }, true);  // 2 - Current


      //TODO: we need more error checking all around this layer to soem.
      // This is just an example of how to fetch error strings from soem
      if (wkc != 1){
        while (ecx_iserror(context) != 0U){
          printf(ecx_elist2string(context));
        }
      }
    }


    // This is can be used to restore default parameters
    //ecx::sdo_write<uint32_t>(context, slave, { 0x1011, 0x01 }, 0x64616F6C );  // RESTORE PARAMETERS TO DELIVERY STATE
    return 1;
  }

  void process_data(uint8_t* input, uint8_t*) noexcept final {
    // Cast pointer type to uint16_t
    bool chg = false;
    auto* in = reinterpret_cast<uint16_t*>(input);  // NOLINT
    for (size_t i = 0; i < size; i++) {
      auto temp = *in++;
      if ((status_[i] & 0x00ff) != (temp & 0x00ff)) {  // Don't compare the expdo_x
        chg = true;
      }
      status_[i] = temp;
      temp = *in++;
      if (value_[i] != temp)
        chg = true;
      value_[i] = temp;
    }

    if (chg) {
      printf("\nValue: ");
      for (auto& val : value_) {
        printf("0x%x\t", val);
      }
      printf("\n");

      printf("Status:\n");
      for (auto& val : status_) {
        auto status_parse = std::bitset<16>(val);
        bool const under_range = status_parse.test(0);
        bool const over_range = status_parse.test(1);
        bool const limit_1 = status_parse.test(2);
        bool const limit_2 = status_parse.test(3);
        bool const error = status_parse.test(4);
        bool const txpdo_state = status_parse.test(14);
        bool const txpdo_toggle = status_parse.test(15);
        printf("%s\t%s\t%s\t%s\t%s\t%s\t%s", debug_print("under_range", under_range).c_str(),
               debug_print("over_range", over_range).c_str(), debug_print("limit_1", limit_1).c_str(),
               debug_print("limit_2", limit_2).c_str(), debug_print("error", error).c_str(),
               debug_print("txpdo_state", txpdo_state).c_str(), debug_print("txpdo_toggle", txpdo_toggle).c_str());
        printf("\n");
      }
    }
  }

private:
  std::array<uint16_t, size> status_;
  std::array<uint16_t, size> value_;
};

using el3054 = el305x<4, 0xbee3052>;
