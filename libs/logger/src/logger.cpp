import std;
import fmt;
import spdlog;
import asio;
import tfc.base;
#include <stdio.h>
#include <tfc/logger.hpp>
#include <tfc/utils/pragmas.hpp>
#include "custom_sink.hpp"

inline constexpr std::string_view logging_pattern = "*** %l [%H:%M:%S %z] (thread %t) {0}.{1} *** \t\t %v ";
inline constexpr std::size_t tp_queue_size = 128;
inline constexpr std::size_t tp_worker_count = 1;

namespace {
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
// clang-format on
std::shared_ptr<spdlog::details::thread_pool> thread_pool;
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
}  // namespace

tfc::logger::logger::logger(std::string_view key) : key_{ key } {
  // Create sinks
  std::shared_ptr<spdlog::sinks::tfc_systemd_sink_mt> systemd;
  try {
    systemd = std::make_shared<spdlog::sinks::tfc_systemd_sink_mt>(key_);
  } catch (std::system_error const& err) {
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
void tfc::logger::logger::log_(lvl_e log_lvl, std::string_view msg) const {
  async_logger_->log(static_cast<spdlog::level::level_enum>(log_lvl), msg);
}
void tfc::logger::logger::set_loglevel(tfc::logger::lvl_e log_level) {
  async_logger_->set_level(static_cast<spdlog::level::level_enum>(log_level));
}
