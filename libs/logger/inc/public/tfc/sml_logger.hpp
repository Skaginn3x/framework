#pragma once

// Standard
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

// Third party
#include <boost/sml.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/concepts.hpp>

namespace tfc::logger {

namespace detail {
template <typename type_t>
concept name_exists = requires {
  { type_t::name };
  requires std::same_as<std::string_view, std::remove_cvref_t<decltype(type_t::name)>>;
};

template <typename type_t>
concept is_sub_sm = tfc::stx::is_specialization_v<type_t, boost::sml::back::sm>;

template <typename type_t>
struct sub_sm {};

// need the inner type to be able to get the name
template <typename type_t>
struct sub_sm<boost::sml::back::sm<boost::sml::back::sm_policy<type_t>>> {
  using type = type_t;
};

template <typename type_t>
constexpr auto get_name() -> std::string {
  if constexpr (is_sub_sm<type_t>) {
    using sub_sm = sub_sm<type_t>;
    return get_name<typename sub_sm::type>();
  } else if constexpr (name_exists<type_t>) {
    return std::string{ type_t::name };
  } else {
    return std::string{ boost::sml::aux::string<type_t>{}.c_str() };
  }
}
namespace test {
struct test_state {
  static constexpr std::string_view name{ "test_state" };
};
struct invalid_test_state {
  static constexpr std::string_view name1{ "test_state" };
};
static_assert(name_exists<test_state>);
static_assert(!name_exists<invalid_test_state>);
}  // namespace test
}  // namespace detail

/**
 * @brief Boost state machine library (SML) logger with tfc::logger instance.
 * */
struct sml_logger {
  sml_logger() = default;

  sml_logger(std::string_view key) : logger_{ std::make_shared<tfc::logger::logger>(key) } {}

  /**
   * @brief log event that happens in the state machine to the logger
   * */
  template <class SM, class TEvent>
  void log_process_event(const TEvent& /*event*/) {  // NOLINT(readability-identifier-naming)
    logger_->trace("[{}][process_event] {}\n", detail::get_name<SM>(), detail::get_name<TEvent>());
  }

  /**
   * @brief log guard that is checked in the state machine, outputs whether the guard is OK or Rejected
   * */
  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard& /*guard*/,
                 const TEvent& /*event*/,
                 bool result) {  // NOLINT(readability-identifier-naming)
    logger_->trace("[{}][guard] {} {} {}\n", detail::get_name<SM>(), detail::get_name<TGuard>(), detail::get_name<TEvent>(),
                   (result ? "[OK]" : "[Reject]"));
  }

  /**
   * @brief log action that is taken in the state machine
   * */
  template <class SM, class TAction, class TEvent>
  void log_action(const TAction& /*action*/, const TEvent& /*event*/) {  // NOLINT(readability-identifier-naming)
    logger_->trace("[{}][action] {} {}\n", detail::get_name<SM>(), detail::get_name<TAction>(), detail::get_name<TEvent>());
  }

  /**
   * @brief log state change that happens in the state machine
   * */
  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {  // NOLINT(readability-identifier-naming)
    logger_->trace("[{}][transition] {} -> {}\n", detail::get_name<SM>(), src.c_str(), dst.c_str());
  }

private:
  std::shared_ptr<tfc::logger::logger> logger_{ std::make_shared<tfc::logger::logger>("sml") };
};
}  // namespace tfc::logger
