#pragma once

#include <memory>
#include <source_location>
#include <string>
#include <string_view>

#include <fmt/core.h>

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
   * @brief Log messages
   * @param msg String to log
   */
  template <lvl_e log_level>
  void log(std::string_view msg, std::source_location loc = std::source_location::current()) const {
    log_(log_level, msg, loc);
  }
  /**
   * @brief Log and format messages
   * @param msg String to log
   * @param p1 Variable embedded into the msg
   * @param loc Source location
   */
  // clang-format off
  template <lvl_e log_level, typename t1>
  void log(fmt::format_string<t1> msg, t1&& p1, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2>
  void log(fmt::format_string<t1, t2> msg, t1&& p1, t2&& p2, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3>
  void log(fmt::format_string<t1, t2, t3> msg, t1&& p1, t2&& p2, t3&& p3, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3, typename t4>
  void log(fmt::format_string<t1, t2, t3, t4> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3, typename t4, typename t5>
  void log(fmt::format_string<t1, t2, t3, t4, t5> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
  void log(fmt::format_string<t1, t2, t3, t4, t5, t6> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
  void log(fmt::format_string<t1, t2, t3, t4, t5, t6, t7> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
  void log(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9>
  void log(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8, t9> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9)), loc);
  }
  template <lvl_e log_level, typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9, typename t10>
  void log(fmt::format_string<t1, t2, t3, t4, t4, t5, t6, t7, t8, t9, t10> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, t10&& p10, std::source_location loc = std::source_location::current()) const {
    log_(log_level, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)), loc);
  }

  void trace(std::string_view msg, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, msg, loc);
  }
  template <typename t1>
  void trace(fmt::format_string<t1> msg, t1&& p1, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1)), loc);
  }
  template <typename t1, typename t2>
  void trace(fmt::format_string<t1, t2> msg, t1&& p1, t2&& p2, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2)), loc);
  }
  template <typename t1, typename t2, typename t3>
  void trace(fmt::format_string<t1, t2, t3> msg, t1&& p1, t2&& p2, t3&& p3, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4>
  void trace(fmt::format_string<t1, t2, t3, t4> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5>
  void trace(fmt::format_string<t1, t2, t3, t4, t5> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
  void trace(fmt::format_string<t1, t2, t3, t4, t5, t6> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
  void trace(fmt::format_string<t1, t2, t3, t4, t5, t6, t7> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
  void trace(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9>
  void trace(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8, t9> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9, typename t10>
  void trace(fmt::format_string<t1, t2, t3, t4, t4, t5, t6, t7, t8, t9, t10> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, t10&& p10, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::trace, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)), loc);
  }

  void debug(std::string_view msg, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, msg, loc);
  }
  template <typename t1>
  void debug(fmt::format_string<t1> msg, t1&& p1, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1)), loc);
  }
  template <typename t1, typename t2>
  void debug(fmt::format_string<t1, t2> msg, t1&& p1, t2&& p2, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2)), loc);
  }
  template <typename t1, typename t2, typename t3>
  void debug(fmt::format_string<t1, t2, t3> msg, t1&& p1, t2&& p2, t3&& p3, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4>
  void debug(fmt::format_string<t1, t2, t3, t4> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5>
  void debug(fmt::format_string<t1, t2, t3, t4, t5> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
  void debug(fmt::format_string<t1, t2, t3, t4, t5, t6> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
  void debug(fmt::format_string<t1, t2, t3, t4, t5, t6, t7> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
  void debug(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9>
  void debug(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8, t9> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9, typename t10>
  void debug(fmt::format_string<t1, t2, t3, t4, t4, t5, t6, t7, t8, t9, t10> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, t10&& p10, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::debug, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)), loc);
  }

  void info(std::string_view msg, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, msg, loc);
  }
  template <typename t1>
  void info(fmt::format_string<t1> msg, t1&& p1, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1)), loc);
  }
  template <typename t1, typename t2>
  void info(fmt::format_string<t1, t2> msg, t1&& p1, t2&& p2, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2)), loc);
  }
  template <typename t1, typename t2, typename t3>
  void info(fmt::format_string<t1, t2, t3> msg, t1&& p1, t2&& p2, t3&& p3, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4>
  void info(fmt::format_string<t1, t2, t3, t4> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5>
  void info(fmt::format_string<t1, t2, t3, t4, t5> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
  void info(fmt::format_string<t1, t2, t3, t4, t5, t6> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
  void info(fmt::format_string<t1, t2, t3, t4, t5, t6, t7> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
  void info(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9>
  void info(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8, t9> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9, typename t10>
  void info(fmt::format_string<t1, t2, t3, t4, t4, t5, t6, t7, t8, t9, t10> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, t10&& p10, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::info, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)), loc);
  }

  void warn(std::string_view msg, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, msg, loc);
  }
  template <typename t1>
  void warn(fmt::format_string<t1> msg, t1&& p1, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1)), loc);
  }
  template <typename t1, typename t2>
  void warn(fmt::format_string<t1, t2> msg, t1&& p1, t2&& p2, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2)), loc);
  }
  template <typename t1, typename t2, typename t3>
  void warn(fmt::format_string<t1, t2, t3> msg, t1&& p1, t2&& p2, t3&& p3, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4>
  void warn(fmt::format_string<t1, t2, t3, t4> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5>
  void warn(fmt::format_string<t1, t2, t3, t4, t5> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
  void warn(fmt::format_string<t1, t2, t3, t4, t5, t6> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
  void warn(fmt::format_string<t1, t2, t3, t4, t5, t6, t7> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
  void warn(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9>
  void warn(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8, t9> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9, typename t10>
  void warn(fmt::format_string<t1, t2, t3, t4, t4, t5, t6, t7, t8, t9, t10> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, t10&& p10, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::warn, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)), loc);
  }

  void error(std::string_view msg, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, msg, loc);
  }
  template <typename t1>
  void error(fmt::format_string<t1> msg, t1&& p1, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1)), loc);
  }
  template <typename t1, typename t2>
  void error(fmt::format_string<t1, t2> msg, t1&& p1, t2&& p2, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2)), loc);
  }
  template <typename t1, typename t2, typename t3>
  void error(fmt::format_string<t1, t2, t3> msg, t1&& p1, t2&& p2, t3&& p3, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4>
  void error(fmt::format_string<t1, t2, t3, t4> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5>
  void error(fmt::format_string<t1, t2, t3, t4, t5> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
  void error(fmt::format_string<t1, t2, t3, t4, t5, t6> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
  void error(fmt::format_string<t1, t2, t3, t4, t5, t6, t7> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
  void error(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9>
  void error(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8, t9> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9, typename t10>
  void error(fmt::format_string<t1, t2, t3, t4, t4, t5, t6, t7, t8, t9, t10> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, t10&& p10, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::error, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)), loc);
  }

  void critical(std::string_view msg, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, msg, loc);
  }
  template <typename t1>
  void critical(fmt::format_string<t1> msg, t1&& p1, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1)), loc);
  }
  template <typename t1, typename t2>
  void critical(fmt::format_string<t1, t2> msg, t1&& p1, t2&& p2, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2)), loc);
  }
  template <typename t1, typename t2, typename t3>
  void critical(fmt::format_string<t1, t2, t3> msg, t1&& p1, t2&& p2, t3&& p3, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4>
  void critical(fmt::format_string<t1, t2, t3, t4> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5>
  void critical(fmt::format_string<t1, t2, t3, t4, t5> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6>
  void critical(fmt::format_string<t1, t2, t3, t4, t5, t6> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7>
  void critical(fmt::format_string<t1, t2, t3, t4, t5, t6, t7> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8>
  void critical(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9>
  void critical(fmt::format_string<t1, t2, t3, t4, t5, t6, t7, t8, t9> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9)), loc);
  }
  template <typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8, typename t9, typename t10>
  void critical(fmt::format_string<t1, t2, t3, t4, t4, t5, t6, t7, t8, t9, t10> msg, t1&& p1, t2&& p2, t3&& p3, t4&& p4, t5&& p5, t6&& p6, t7&& p7, t8&& p8, t9&& p9, t10&& p10, std::source_location loc = std::source_location::current()) const {
    log_(lvl_e::critical, fmt::vformat(msg, fmt::make_format_args(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)), loc);
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
  void log_(lvl_e log_lvl, std::string_view msg, std::source_location loc) const;
  std::string key_;
  std::shared_ptr<spdlog::async_logger> async_logger_;
};
};  // namespace tfc::logger
