#pragma once

#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/track/behaviour.hpp>

#include <tfc/track/actions/generic_io.hpp>
#include <tfc/track/entries/connection.hpp>
#include <tfc/track/entries/sensor_ctrl_motor.hpp>
#include <tfc/track/exits/sensor_ctrl_motor.hpp>

namespace tfc::track {

namespace types {

using actions_t = std::variant<actions::generic_io>;
using entries_t = std::variant<entries::connection, entries::sensor_ctrl_motor>;
using exits_t = std::variant<exits::sensor_ctrl_motor>;

}  // namespace types

namespace asio = boost::asio;

class ctrl {
public:
  ctrl(asio::io_context& io_context) : ctx_{ io_context } {}

private:
  asio::io_context& ctx_;

  struct config {};
};

}  // namespace tfc::track

namespace glz {

template <typename>
struct meta;

template <>
struct meta<tfc::track::types::actions_e> {
  static constexpr std::string_view name{ "actions_e" };
  using enum tfc::track::types::actions_e;
  static constexpr auto value{ glz::enumerate("unknown", unknown, "generic_io", generic_io) };
};

template <>
struct meta<tfc::track::types::entries_e> {
  static constexpr std::string_view name{ "entries_e" };
  using enum tfc::track::types::entries_e;
  static constexpr auto value{
    glz::enumerate("unknown", unknown, "connection", connection, "sensor_ctrl_motor", sensor_ctrl_motor)
  };
};

template <>
struct meta<tfc::track::types::exits_e> {
  static constexpr std::string_view name{ "exits_e" };
  using enum tfc::track::types::exits_e;
  static constexpr auto value{ glz::enumerate("unknown", unknown, "sensor_ctrl_motor", sensor_ctrl_motor) };
};

}  // namespace glz
