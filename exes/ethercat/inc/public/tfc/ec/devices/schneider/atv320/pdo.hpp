#include <tfc/ec/devices/schneider/atv320/settings.hpp>
#pragma once
namespace tfc::ec::devices::schneider::atv320 {

struct input_t {
  tfc::ec::cia_402::status_word status_word{};
  decifrequency_signed frequency{};
  uint16_t current{};
  uint16_t digital_inputs{};
  lft_e last_error;
  hmis_e drive_state{};
};

struct output_t {
  cia_402::control_word control{};
  decifrequency_signed frequency{};
  uint16_t digital_outputs{};
  deciseconds acc{};
  deciseconds dec{};
};

static_assert(sizeof(input_t) == 12);
static_assert(sizeof(output_t) == 10);

}  // namespace tfc::ec::devices::schneider::atv320
