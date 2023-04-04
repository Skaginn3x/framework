#pragma once

#include <units/isq/si/electric_current.h>
#include <units/isq/si/thermodynamic_temperature.h>
#include <units/quantity_point.h>
#include <units/unit.h>

#include <concepts>

template <typename from, typename to>
auto map(from value, from in_min, from in_max, to out_min, to out_max) -> to {
  return static_cast<to>((static_cast<double>(value - in_min)) * static_cast<double>(out_max - out_min) /
                             static_cast<double>(in_max - in_min) +
                         static_cast<double>(out_min));
}
