#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <glaze/core/common.hpp>

struct signal_name {
  std::string value{};
};

static inline std::vector<std::string> get_available_signal_names() {
  boost::asio::io_context ctx{};
  tfc::ipc_ruler::ipc_manager_client client{ ctx };
  std::vector<std::string> names{};
  client.signals([&names](std::vector<tfc::ipc_ruler::signal> const& signals) {
    for (auto const& signal : signals) {
      names.push_back(signal.name);
    }
  });
  ctx.run_for(std::chrono::milliseconds{ 50 });
  return names;
}

template <>
struct tfc::json::detail::to_json_schema<signal_name> {
  template <auto Opts>
  static void op(auto& s, auto& defs) noexcept {
    for (auto const& name : get_available_signal_names()) {
      s.oneOf.push_back(tfc::json::detail::schematic{ .attributes{ tfc::json::schema{ .title = name } } });
    };
  }
};

namespace tfc::mqtt::config {

struct publish_signals {
  std::vector<std::string> publish_signals{};

  struct glaze {
    static constexpr auto value{ glz::object("publish_signals", &publish_signals::publish_signals, "Signals to publish") };
    static constexpr std::string_view name{ "publish_signals" };
  };
};

}  // namespace tfc::mqtt::config
