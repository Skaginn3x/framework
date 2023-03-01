#pragma once

#include <fmt/core.h>
#include <memory>
#include <string>
#include <string_view>

namespace spdlog {
class async_logger;
namespace details {
class thread_pool;
}
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
  /**
   * @brief Log and format messages
   * @param msg String to log
   * @param parameters Variables embedded into the msg
   */
  template <lvl_e T, typename... P>
  void log(fmt::format_string<P...> msg, P&&... parameters) {
    log_(T, fmt::vformat(msg, fmt::make_format_args(parameters...)));
  }

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
  void log_(lvl_e log_lvl, std::string_view msg);
  std::string key_;
  std::shared_ptr<spdlog::async_logger> async_logger_;
  std::shared_ptr<spdlog::details::thread_pool> tp_;
};
};  // namespace tfc::logger
