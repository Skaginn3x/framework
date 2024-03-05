#pragma once

#include <tfc/ec/devices/lenze/i550/settings.hpp>

namespace tfc::ec::devices::lenze::i550 {

struct input_t {
  cia_402::status_word status_word{};
  std::uint16_t rpm{};
  std::uint16_t error{};

  //decifrequency_signed frequency{};
  /*uint16_t current{};
  uint16_t digital_inputs{};
  lft_e last_error;
  hmis_e drive_state{};*/
};

struct output_t {
  cia_402::control_word control{};
  std::uint16_t rpm{};
  /*decifrequency_signed frequency{};
  uint16_t digital_outputs{};
  deciseconds acc{};
  deciseconds dec{};*/
};

//static_assert(sizeof(input_t) == 12);
//static_assert(sizeof(output_t) == 10);

}  // namespace tfc::ec::devices::lenze::i550

