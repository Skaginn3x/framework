#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <glaze/core/common.hpp>
#include <tfc/utils/json_schema.hpp>

#include <signal_names.hpp>

namespace tfc::mqtt::config {

struct signal_name;

std::vector<std::string> publishing_signals{};

struct signal_name {
  signal_name() { publishing_signals.push_back(*this); }

  std::string value{};

  struct glaze {
    static constexpr auto value = &signal_name::value;
    static constexpr std::string_view name{ "signal_name" };
  };
};

}  // namespace tfc::mqtt::config

auto signal_in_publishing_list(std::string signal_name) {
  for (auto const& signal : tfc::mqtt::config::publishing_signals) {
    if (signal.value == signal_name) {
      return true;
    }
  }
  return false;
}

template <>
struct tfc::json::detail::to_json_schema<tfc::mqtt::config::signal_name> {
  template <auto Opts>
  static void op(auto& s, auto&) noexcept {
    if (!s.oneOf.has_value()) {
      s.oneOf = std::vector<tfc::json::detail::schematic>{};
    }
    for (auto const& signal : tfc::global::get_signals()) {
      if (signal_in_publishing_list(signal.name)) {
        continue;
      }

      s.oneOf.value().push_back(tfc::json::detail::schematic{ .attributes{
          tfc::json::schema{ .title = signal.name, .description = signal.description, .constant = signal.name } } });
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
