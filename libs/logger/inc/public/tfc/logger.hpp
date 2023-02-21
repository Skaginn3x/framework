#pragma once

#include <string_view>

namespace tfc::logger{
enum lvl {
  info,
  warn,
  error,
  critical,
  trace,
};
/**
 * spdlog wrapper interface, implementing key identification in accordance with
 * internal specification. V2
 */
class logger {

public:
  logger(std::string_view key);

  /** \brief Log method with support to format using fmt::format
  * \note If running as a service journalctl will
  *  recieve log output. --cmdline can also be
   *  added so that the output comes out both in journalctl and
   *  in the cli
  *
  * \param msg string to log
   */
  template <lvl T, typename... P>
  void log(std::string_view msg, P&& ...parameters) noexcept;
};
};
