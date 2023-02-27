#pragma once

// Copyright(c) 2019 ZVYAGIN.Alexander@gmail.com
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/base_sink.h>

#include <array>
#ifndef SD_JOURNAL_SUPPRESS_LOCATION
#define SD_JOURNAL_SUPPRESS_LOCATION
#endif
#include <systemd/sd-journal.h>
#include "tfc/progbase.hpp"

namespace spdlog {
namespace sinks {

/**
 * Sink that write to systemd journal using the `sd_journal_send()` library call.
 */
template <typename mutex>
class tfc_systemd_sink : public base_sink<mutex> {
public:
  explicit tfc_systemd_sink(std::string key, bool enable_formatting = false)
      : key_{std::move(key)}, enable_formatting_{enable_formatting}, syslog_levels_{
                                                                         {/* spdlog::level::trace      */ LOG_DEBUG,
                                                                          /* spdlog::level::debug      */ LOG_DEBUG,
                                                                          /* spdlog::level::info       */ LOG_INFO,
                                                                          /* spdlog::level::warn       */ LOG_WARNING,
                                                                          /* spdlog::level::err        */ LOG_ERR,
                                                                          /* spdlog::level::critical   */ LOG_CRIT,
                                                                          /* spdlog::level::off        */ LOG_INFO}} {}

  ~tfc_systemd_sink() override = default;

  tfc_systemd_sink(const tfc_systemd_sink&) = delete;
  auto operator=(const tfc_systemd_sink&) -> tfc_systemd_sink& = delete;

protected:
  const std::string key_;
  bool enable_formatting_ = false;
  using levels_array = std::array<int, 7>;
  levels_array syslog_levels_;

  void sink_it_(const details::log_msg& msg) override {
    int err;
    string_view_t payload;
    memory_buf_t formatted;
    if (enable_formatting_) {
      base_sink<mutex>::formatter_->format(msg, formatted);
      payload = string_view_t(formatted.data(), formatted.size());
    } else {
      payload = msg.payload;
    }

    size_t length = payload.size();
    // limit to max int
    if (length > static_cast<size_t>(std::numeric_limits<int>::max())) {
      length = static_cast<size_t>(std::numeric_limits<int>::max());
    }

    const string_view_t tfc_exe = tfc::base::get_exe_name();
    const string_view_t tfc_proc_name = tfc::base::get_proc_name();

    // Do not send source location if not available
    if (msg.source.empty()) {
      // Note: function call inside '()' to avoid macro expansion
      err = (sd_journal_send)("MESSAGE=%.*s", static_cast<int>(length), payload.data(), "PRIORITY=%d",
                              syslog_level(msg.level), "TFC_KEY=%.*s", static_cast<int>(key_.size()), key_.data(),
                              "TFC_EXE=%.*s", static_cast<int>(tfc_exe.size()), tfc_exe.data(), "TFC_ID=%.*s",
                              static_cast<int>(tfc_proc_name.size()), tfc_proc_name.data(), nullptr);
    } else {
      err = (sd_journal_send)("MESSAGE=%.*s", static_cast<int>(length), payload.data(), "PRIORITY=%d",
                              syslog_level(msg.level), "CODE_FILE=%s", msg.source.filename, "CODE_LINE=%d", msg.source.line,
                              "CODE_FUNC=%s", msg.source.funcname, "TFC_KEY=%.*s", static_cast<int>(key_.size()),
                              key_.data(), "TFC_EXE=%.*s", static_cast<int>(tfc_exe.size()), tfc_exe.data(), "TFC_ID=%.*s",
                              static_cast<int>(tfc_proc_name.size()), tfc_proc_name.data(), nullptr);
    }

    if (err) {
      throw_spdlog_ex("Failed writing to systemd", errno);
    }
  }

  auto syslog_level(level::level_enum lvl) -> int { return syslog_levels_.at(static_cast<levels_array::size_type>(lvl)); }

  void flush_() override {}
};

using tfc_systemd_sink_mt = tfc_systemd_sink<std::mutex>;
using tfc_systemd_sink_st = tfc_systemd_sink<details::null_mutex>;
}  // namespace sinks

// Create and register a syslog logger
template <typename factory = spdlog::synchronous_factory>
inline auto systemd_logger_mt(const std::string& logger_name, const std::string& key = "", bool enable_formatting = false)
    -> std::shared_ptr<logger> {
  return factory::template create<sinks::tfc_systemd_sink_mt>(logger_name, key, enable_formatting);
}

template <typename factory = spdlog::synchronous_factory>
inline auto systemd_logger_st(const std::string& logger_name, const std::string& key = "", bool enable_formatting = false)
    -> std::shared_ptr<logger> {
  return factory::template create<sinks::tfc_systemd_sink_st>(logger_name, key, enable_formatting);
}
}  // namespace spdlog
