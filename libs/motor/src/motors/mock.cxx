module;
#include <boost/asio/io_context.hpp>
#include <glaze/core/common.hpp>
#include <string>
#include <string_view>
#include <tfc/confman/observable.hpp>
#include <tfc/logger.hpp>

export module motor:mock;

namespace tfc::motor::types {
using tfc::confman::observable;
export class virtual_motor {
private:
  struct config {
    using impl = virtual_motor;
    observable<std::string> name;
    struct glaze {
      using T = config;
      static constexpr auto value = glz::object("name", &T::name);
      static constexpr std::string_view name{ "printing_motor" };
    };
    auto operator==(const config&) const noexcept -> bool = default;
  };

public:
  using config_t = config;
  explicit virtual_motor(boost::asio::io_context&, const config& config) : config_(config), logger_(config.name.value()) {
    logger_.info("virtual_motor c-tor: {}", config.name.value());
    config_.name.observe([this](std::string const& new_v, std::string const& old_v) {
      logger_.warn("Printing motor name switched from: {}, to: {}! takes effect after this short message", old_v, new_v);
      logger_ = logger::logger(new_v);
    });
  }
  ~virtual_motor(){
    logger_.info("virtual_motor d-tor: {}", config_.name.value());
  }
  void pump() { logger_.info("pump!"); }

private:
  const config_t& config_;
  tfc::logger::logger logger_;
};
}  // namespace tfc::motor::types
