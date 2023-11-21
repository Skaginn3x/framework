#pragma once

#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/ipc.hpp>

namespace tfc::track::actions {

namespace asio = boost::asio;

class generic_io {
public:
  struct config {
    bool stop_motor{ false };
  };


  generic_io(asio::io_context& io_context, config& cfg)
    : ctx_{ io_context }, cfg_{ cfg }
  {}


private:
  asio::io_context& ctx_;
  config& cfg_;
};


} // namespace tfc::track::actions

namespace glz {

template <typename>
struct meta;

template <>
struct meta<tfc::track::actions::generic_io::config> {
  using type = tfc::track::actions::generic_io::config;
  static constexpr std::string_view name{ "generic_io" };
  static constexpr auto value{ glz::object("stop_motor", &type::stop_motor, "Stop motor while awaiting complete indicator") };
};

}
