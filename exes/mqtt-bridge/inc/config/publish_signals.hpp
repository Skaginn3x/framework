#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <glaze/core/common.hpp>
#include <tfc/utils/json_schema.hpp>

#include <tfc/utils/pragmas.hpp>

#include <signal_names.hpp>

namespace tfc::mqtt::config {

struct signal_name {
  std::string value{};

  struct glaze {
    static constexpr auto value = &signal_name::value;
    static constexpr std::string_view name{ "signal_name" };
  };
};
}  // namespace tfc::mqtt::config

template <>
struct tfc::json::detail::to_json_schema<tfc::mqtt::config::signal_name> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    s.oneOf = std::vector<schematic>{};
    for (auto const& signal : global::get_signals()) {
      // clang-format off
      PRAGMA_GCC_WARNING_OFF(-Wmaybe-uninitialized)
      // clang-format on
      s.oneOf->emplace_back(schematic{
          .attributes{ schema{ .title = signal.name, .description = signal.description, .constant = signal.name } } });
      PRAGMA_GCC_WARNING_POP
    };
  }
};

namespace tfc::mqtt::config {

struct publish_signals {
  std::vector<signal_name> publish_signals{};

  struct glaze {
    static constexpr auto value{ glz::object("publish_signals", &publish_signals::publish_signals, "Signals to publish") };
    static constexpr std::string_view name{ "publish_signals" };
  };
};

}  // namespace tfc::mqtt::config
