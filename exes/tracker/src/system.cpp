#include "system.h"

namespace tfc::tracker {

system::system(boost::asio::io_context& ctx)
    : ctx_{ ctx }, config_{ ctx_, "system", system_config{ .conveyor_instances = std::vector<std::string>{ "default" } } } {

}
auto system::make_conveyors() -> void {
  for (auto const& instance : config_->conveyor_instances) {

  }
}

}  // namespace tfc::tracker
