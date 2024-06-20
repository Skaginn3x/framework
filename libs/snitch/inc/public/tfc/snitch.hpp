#pragma once

#include <cstdint>
#include <string_view>

#include <fmt/core.h>
#include <fmt/args.h>

#include <tfc/logger.hpp>
#include <tfc/stx/concepts.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/snitch/common.hpp>
#include <tfc/snitch/format_extension.hpp>
#include <tfc/snitch/details/dbus_client.hpp>

namespace tfc::snitch {

struct variance {
  bool requires_acknowledgement{};
  level_e lvl{ level_e::unknown };
};

template <typename T>
concept named_arg = requires(T t) {
  { t.name } -> std::convertible_to<const char*>;
  { t.value };
};

template <variance var, stx::basic_fixed_string in_description, stx::basic_fixed_string in_details>
class alarm {
public:
  static constexpr std::string_view description{ in_description };
  static constexpr std::string_view details{ in_details };
  static constexpr auto description_arg_keys{ detail::arg_names<description>() };
  static constexpr auto details_arg_keys{ detail::arg_names<details>() };
  static constexpr auto keys_count{ std::max(description_arg_keys.size(), details_arg_keys.size()) };

  /// \example alarm<{}, "short desc", "long desc"> warn(fmt::arg("key", "value"), fmt::arg("key2", 1));
  /// \param unique_id A unique identifier for the alarm within this process
  /// \param default_args Default fmt::arg values to populate the alarm with, e.g. fmt::arg("index", 1) for tank 1 etc.
  alarm(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view unique_id, named_arg auto&& ... default_args) : default_values_{ std::make_pair(default_args.name, fmt::format("{}", default_args.value))... } {
    static_assert(detail::check_all_arguments_named(description), "All arguments must be named");
    static_assert(detail::check_all_arguments_no_format(description), "All arguments may not have format specifiers");
    [[maybe_unused]] static constexpr int num_args = sizeof...(default_args);
    static_assert(num_args <= keys_count, "Too many default arguments");

  }

  template <stx::invocable callback_t>
    requires (var.requires_acknowledgement)
  void on_ack(callback_t&& callback) {
  }

  void set(named_arg auto&& ... args) {
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    for (auto const& [key, value] : default_values_) {
      store.push_back(fmt::arg(key.c_str(), value));
    }
    (store.push_back(args), ...);
    logger_.debug(fmt::vformat(description, store));
    logger_.debug(fmt::vformat(details, store));
  }
private:
  std::optional<api::alarm_id_t> alarm_id_{};
  std::string my_name_;
  std::unordered_map<std::string, std::string> default_values_;
  std::shared_ptr<sdbusplus::asio::connection> conn_;
  detail::dbus_client dbus_client_{ conn_ };
  logger::logger logger_{ fmt::format("snitch.{}", my_name_) };
};

template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using info = alarm<variance{ .requires_acknowledgement = false, .lvl = level_e::info }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning = alarm<variance{ .requires_acknowledgement = false, .lvl = level_e::warning }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning_latched = alarm<variance{ .requires_acknowledgement = true, .lvl = level_e::warning }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning_ack = warning_latched<description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using error = alarm<variance{ .requires_acknowledgement = false, .lvl = level_e::error }, description, details>;


} // namespace tfc::snitch
