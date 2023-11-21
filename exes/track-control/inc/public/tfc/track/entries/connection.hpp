#pragma once


#include <boost/asio.hpp>
#include <glaze/core/common.hpp>

#include <tfc/ipc.hpp>

namespace tfc::track::entries {
namespace asio = boost::asio;

class connection {
public:
  struct config {
    static constexpr std::string_view entry_type{ "connection" };
    std::uint64_t queue_size{ 1 };
  };

  connection(asio::io_context& io_context, config& cfg)
    : ctx_{ io_context }, cfg_{ cfg } {
  }

private:
  asio::io_context& ctx_;
  config& cfg_;
};
} // namespace tfc::track::actions

namespace glz {
template <typename>
struct meta;

template <>
struct meta<tfc::track::entries::connection::config> {
  using type = tfc::track::entries::connection::config;
  static constexpr std::string_view name{ "generic_io" };
  static constexpr auto value{ glz::object("entry_type", &type::entry_type, "Const name of this type of entry",
                                           "queue_size", &type::queue_size,
                                           tfc::json::schema{
                                             .description =
                                             "Size of the queue for this entry, how many items can be queued up before blocking",
                                             .minimum = 1UL }
      ) };
};
}
