#pragma once

// Standard
#include <memory>

// Third party
#include <fmt/core.h>
#include <boost/sml.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <tfc/logger.hpp>

namespace spdlog {
class async_logger;
}  // namespace spdlog

namespace tfc::logger {
/*! Logging level*/
enum struct lvl_e : int {
  trace = 0,
  debug = 1,
  info = 2,
  warn = 3,
  error = 4,
  critical = 5,
  off = 6, /*! Log regardless of set logging level*/
};

/**
 * @brief tfc::logger class used for transmitting log messages with id aquired from tfc::base and keys from project
 * components see @example logging_example.cpp for how to use this class.
 * */
class logger {
public:
  /**
   * Create a new logger object with your components key
   * @brief Constructor
   * @param key The components key. f.e. "conveyor-left"
   * */
  explicit logger(std::string_view key);

  // delete copy constructor and copy assignment
  logger(logger const&) = delete;

  auto operator=(logger const&) -> logger = delete;

  // default move constructors
  logger(logger&&) = default;

  auto operator=(logger&&) -> logger& = default;

  /**
   * @brief Log and format messages
   * @param msg String to log
   * @param parameters Variables embedded into the msg
   */
  template <lvl_e log_level, typename... args_t>
  void log(fmt::format_string<args_t...> msg, args_t&&... parameters) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(parameters...)));
  }

  /**
   * @brief Log messages
   * @param msg String to log
   */
  template <lvl_e log_level>
  void log(std::string_view msg) const {
    log_(log_level, msg);
  }

  template <typename... args_t>
  void trace(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<lvl_e::trace>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }

  void trace(std::string_view msg) const { log<lvl_e::trace>(msg); }

  template <typename... args_t>
  void debug(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<lvl_e::debug>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }

  void debug(std::string_view msg) const { log<lvl_e::debug>(msg); }

  template <typename... args_t>
  void info(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<lvl_e::info>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }

  void info(std::string_view msg) const { log<lvl_e::info>(msg); }

  template <typename... args_t>
  void warn(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<lvl_e::warn>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }

  void warn(std::string_view msg) const { log<lvl_e::warn>(msg); }

  template <typename... args_t>
  void error(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<lvl_e::error>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }

  void error(std::string_view msg) const { log<lvl_e::error>(msg); }

  template <typename... args_t>
  void critical(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<lvl_e::critical>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }

  void critical(std::string_view msg) const { log<lvl_e::critical>(msg); }

  /**
   * @brief Override loglevel set by program parameters
   * @param log_level new log level
   * */
  void set_loglevel(lvl_e log_level);

private:
  /**
   * @brief Log messages
   * @param log_lvl Log level
   * @param msg String to log
   */
  void log_(lvl_e log_lvl, std::string_view msg) const;

  std::string key_;
  std::shared_ptr<spdlog::async_logger> async_logger_;
};

/**
 * @brief tfc::logger class used to implement a logger for the Boost SML library, can be used in projects that implement
 * state machines
 * */
struct SmlLogger {
  /**
   * @brief log event that happens in the state machine to the logger
   * */
  template <class SM, class TEvent>
  void log_process_event(const TEvent& /*event*/) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][process_event] {}\n", boost::sml::aux::get_type_name<SM>(),
                                           boost::sml::aux::get_type_name<TEvent>());
  }

  /**
   * @brief log guard that is checked in the state machine, outputs whether the guard is OK or Rejected
   * */
  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard& /*guard*/,
                 const TEvent& /*event*/,
                 bool result) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][guard] {} {} {}\n", boost::sml::aux::get_type_name<SM>(),
                                           boost::sml::aux::get_type_name<TGuard>(),
                                           boost::sml::aux::get_type_name<TEvent>(), (result ? "[OK]" : "[Reject]"));
  }

  /**
   * @brief log action that is taken in the state machine
   * */
  template <class SM, class TAction, class TEvent>
  void log_action(const TAction& /*action*/, const TEvent& /*event*/) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][action] {} {}\n", boost::sml::aux::get_type_name<SM>(),
                                           boost::sml::aux::get_type_name<TAction>(),
                                           boost::sml::aux::get_type_name<TEvent>());
  }

  /**
   * @brief log state change that happens in the state machine
   * */
  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {  // NOLINT(readability-identifier-naming)
    logger_->log<tfc::logger::lvl_e::info>("[{}][transition] {} -> {}\n", boost::sml::aux::get_type_name<SM>(), src.c_str(),
                                           dst.c_str());
  }

private:
  std::shared_ptr<tfc::logger::logger> logger_{ std::make_shared<tfc::logger::logger>("sml") };
};
};  // namespace tfc::logger
