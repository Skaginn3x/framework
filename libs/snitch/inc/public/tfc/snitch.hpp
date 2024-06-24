#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string_view>

#include <fmt/args.h>
#include <fmt/core.h>

#include <tfc/snitch/details/snitch_impl.hpp>
#include <tfc/snitch/common.hpp>
#include <tfc/snitch/format_extension.hpp>
#include <tfc/stx/basic_fixed_string.hpp>
#include <tfc/stx/concepts.hpp>

namespace tfc::snitch {

namespace asio = boost::asio;

struct variance {
  bool resettable{};
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
  alarm(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view unique_id, named_arg auto&&... default_args)
      : impl_{ conn, unique_id, description, details, var.resettable, var.lvl, { std::make_pair(default_args.name, fmt::format("{}", default_args.value))... } }
  {
    static_assert(detail::check_all_arguments_named(description), "All arguments must be named, e.g. {name}");
    static_assert(detail::check_all_arguments_no_format(description), "All arguments may not have format specifiers");
    [[maybe_unused]] static constexpr int num_args = sizeof...(default_args);
    static_assert(num_args <= keys_count, "Too many default arguments");
  }
  alarm(alarm const&) = delete;
  alarm& operator=(alarm const&) = delete;
  alarm(alarm&&) = delete;
  alarm& operator=(alarm&&) = delete;
  ~alarm() = default;

  template <stx::invocable callback_t>
    requires(var.resettable)
  void on_try_reset(callback_t&& callback) {
    impl_.on_try_reset(std::forward<callback_t>(callback));
  }

  void set(named_arg auto&&... args) {
    set([](auto){}, std::forward<decltype(args)>(args)...);
  }

  void set(std::function<void(std::error_code)> on_set_finished, named_arg auto&&... args) {
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    for (auto const& [key, value] : impl_.default_values()) {
      store.push_back(fmt::arg(key.c_str(), value));
    }
    (store.push_back(args), ...);
    std::string description_formatted = fmt::vformat(description, store);
    std::string details_formatted = fmt::vformat(details, store);
    impl_.set(description_formatted, details_formatted, { std::make_pair(args.name, fmt::format("{}", args.value))... }, std::move(on_set_finished));
  }

  void reset(std::function<void(std::error_code)> on_reset_finished = [](auto){}) {
    impl_.reset(std::move(on_reset_finished));
  }

  auto alarm_id() const noexcept -> std::optional<api::alarm_id_t> {
    return impl_.alarm_id();
  }

  auto activation_id() const noexcept -> std::optional<api::activation_id_t> {
    return impl_.activation_id();
  }


private:
  detail::alarm_impl impl_;
};



template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using info = alarm<variance{ .resettable = false, .lvl = level_e::info }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning = alarm<variance{ .resettable = false, .lvl = level_e::warning }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning_latched = alarm<variance{ .resettable = true, .lvl = level_e::warning }, description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using warning_resettable = warning_latched<description, details>;
template <stx::basic_fixed_string description, stx::basic_fixed_string details = "">
using error = alarm<variance{ .resettable = true, .lvl = level_e::error }, description, details>;

}  // namespace tfc::snitch
