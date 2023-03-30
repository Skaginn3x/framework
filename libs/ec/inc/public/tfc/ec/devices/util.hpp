#pragma once

#include <units/isq/si/electric_current.h>
#include <units/isq/si/thermodynamic_temperature.h>
#include <units/quantity_point.h>
#include <units/unit.h>

#include <concepts>

template <typename from, typename to>
auto map(from value, from in_min, from in_max, to out_min, to out_max) -> to {
  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}