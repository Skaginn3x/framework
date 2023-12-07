#include <tfc/ec/devices/schneider/atv320/settings.hpp>
#pragma once
namespace tfc::ec::devices::schneider::atv320 {

struct input_t {
  tfc::ec::cia_402::status_word status_word{};
  int16_t frequency{};
  uint16_t current{};
  uint16_t digital_inputs{};
  int16_t analog_input{};
  uint16_t drive_state{};
};
static_assert(sizeof(input_t) == 12);
struct output_t {
  cia_402::control_word control{};
  decifrequency frequency{};
  uint16_t digital_outputs{};
};
static_assert(sizeof(output_t) == 6);

static_assert(sizeof(input_t) == 12);
static_assert(sizeof(output_t) == 6);

}  // namespace tfc::ec::devices::schneider::atv320