module;

import tfc.progbase;

#include <string>
#include <string_view>

#include <fmt/core.h>
#include <memory>

#include "custom_sink.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <tfc/utils/pragmas.hpp>

export module tfc.logger;

namespace tfc::logger {

inline constexpr std::string_view logging_pattern = "*** %l [%H:%M:%S %z] (thread %t) {0}.{1} *** \t\t %v ";
inline constexpr size_t tp_queue_size = 128;
inline constexpr size_t tp_worker_count = 1;

PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
std::shared_ptr<spdlog::details::thread_pool> thread_pool;
PRAGMA_CLANG_WARNING_POP

/**
 * @brief tfc::logger class used for transmitting log messages with id aquired from tfc::base and keys from project
 * components see @example logging_example.cpp for how to use this class.
 * */
export class logger {
public:
  /**
   * Create a new logger object with your components key
   * @brief Constructor
   * @param key The components key. f.e. "conveyor-left"
   * */
  explicit logger(std::string_view key){
    // Create sinks
    std::shared_ptr<spdlog::sinks::tfc_systemd_sink_mt> systemd;
    try {
      systemd = std::make_shared<spdlog::sinks::tfc_systemd_sink_mt>(key_);
    } catch (boost::system::system_error const& err) {
      auto loc = std::source_location::current();
      fmt::print(
          stderr,
          "Unable to open journald socket for logging. using console err: {}, source location: FILE: {}, FUNC: {}, LINE: {}",
          err.what(), loc.file_name(), loc.function_name(), loc.line());
      systemd = nullptr;
    }
    std::vector<spdlog::sink_ptr> sinks{};
    if (systemd != nullptr) {
      sinks.emplace_back(systemd);
    }
    if (tfc::base::is_stdout_enabled() || systemd == nullptr) {
      auto stdout_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

      // customize formatting for stdout messages
      stdout_sink->set_pattern(fmt::format(logging_pattern, tfc::base::get_proc_name(), key));
      sinks.emplace_back(stdout_sink);
    }

    // Wrap them into an async logger
    if (!thread_pool) {
      thread_pool = std::make_shared<spdlog::details::thread_pool>(tp_queue_size, tp_worker_count);
    }

    async_logger_ = std::make_shared<spdlog::async_logger>("async_logger", sinks.begin(), sinks.end(), thread_pool,
                                                           spdlog::async_overflow_policy::overrun_oldest);
    async_logger_->set_level(static_cast<spdlog::level::level_enum>(tfc::base::get_log_lvl()));
  }

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
  template <base::lvl_e log_level, typename... args_t>
  void log(fmt::format_string<args_t...> msg, args_t&&... parameters) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(parameters...)));
  }
  /**
   * @brief Log messages
   * @param msg String to log
   */
  template <base::lvl_e log_level>
  void log(std::string_view msg) const {
    log_(log_level, msg);
  }
  template <typename... args_t>
  void trace(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<base::lvl_e::trace>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }
  void trace(std::string_view msg) const { log<base::lvl_e::trace>(msg); }
  template <typename... args_t>
  void debug(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<base::lvl_e::debug>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }
  void debug(std::string_view msg) const { log<base::lvl_e::debug>(msg); }
  template <typename... args_t>
  void info(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<base::lvl_e::info>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }
  void info(std::string_view msg) const { log<base::lvl_e::info>(msg); }
  template <typename... args_t>
  void warn(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<base::lvl_e::warn>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }
  void warn(std::string_view msg) const { log<base::lvl_e::warn>(msg); }
  template <typename... args_t>
  void error(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<base::lvl_e::error>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }
  void error(std::string_view msg) const { log<base::lvl_e::error>(msg); }
  template <typename... args_t>
  void critical(fmt::format_string<args_t...>&& msg, args_t&&... parameters) const {
    log<base::lvl_e::critical>(std::forward<decltype(msg)>(msg), std::forward<args_t>(parameters)...);
  }
  void critical(std::string_view msg) const { log<base::lvl_e::critical>(msg); }

  /**
   * @brief Override loglevel set by program parameters
   * @param log_level new log level
   * */
  void set_loglevel(base::lvl_e log_level){
    async_logger_->set_level(static_cast<spdlog::level::level_enum>(log_level));
  }

private:
  /**
   * @brief Log messages
   * @param log_lvl Log level
   * @param msg String to log
   */
  void log_(base::lvl_e log_lvl, std::string_view msg) const{
    async_logger_->log(static_cast<spdlog::level::level_enum>(log_lvl), msg);
  }
  std::string key_;
  std::shared_ptr<spdlog::async_logger> async_logger_;
};
};  // namespace tfc::logger
