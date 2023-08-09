#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <unordered_map>

#include <glaze/core/common.hpp>
#include <boost/asio/io_context.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include "conveyor.h"

namespace tfc::tracker {

namespace asio = boost::asio;

struct conveyor_instance {
  std::string name{};
  conveyor conveyor;
};

struct system_config {
  tfc::confman::observable<std::vector<std::string>> conveyor_instances{ {"default"} };
  struct glaze {
    static constexpr std::string_view name{ "system_config" };
    static constexpr auto value{ glz::object("conveyor_instances", &system_config::conveyor_instances) };
  };
};

class system {
public:
  explicit system(asio::io_context&);

private:
  auto make_conveyors() -> void;

  asio::io_context& ctx_;
  tfc::confman::config<system_config> config_;
  std::unordered_map<std::string, conveyor> conveyors_{};
};

}  // namespace tfc::tracker
