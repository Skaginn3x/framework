#include "tfc/logger.hpp"
#include "tfc/progbase.hpp"
#include "custom_sink.hpp"

#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

inline constexpr std::string_view LOGGING_PATTERN = "*** [%H:%M:%S %z] (thread %t) {} *** \n\t %v ";
inline constexpr size_t TP_QUEUE_SIZE             = 128;
inline constexpr size_t TP_WORKER_COUNT           = 1;

tfc::logger::logger::logger(std::string_view key) : key_{key }{
  // Create sinks
  std::vector<spdlog::sink_ptr> sinks { std::make_shared<spdlog::sinks::tfc_systemd_sink_mt>(key_) };
  if (tfc::base::is_stdout_enabled()) {
    auto stdout_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

    // customize formatting for stdout messages
    stdout_sink->set_pattern(fmt::format(LOGGING_PATTERN, tfc::base::get_proc_name()));
    sinks.emplace_back(stdout_sink);
  }

  // Wrap them into an async logger
  tp_ = std::make_shared<spdlog::details::thread_pool>(TP_QUEUE_SIZE, TP_WORKER_COUNT);

  async_logger_ = std::make_shared<spdlog::async_logger>("async_logger", sinks.begin(), sinks.end(), tp_, spdlog::async_overflow_policy::overrun_oldest);
  async_logger_->set_level(static_cast<spdlog::level::level_enum>(tfc::base::get_log_lvl()));
  async_logger_->info("tfc::logger initialized");
}
void tfc::logger::logger::log_(lvl_e log_lvl, std::string_view msg) {
  async_logger_->log(static_cast<spdlog::level::level_enum>(log_lvl), msg);
}
void tfc::logger::logger::set_loglevel(tfc::logger::lvl_e log_level) {
  async_logger_->set_level(static_cast<spdlog::level::level_enum>(log_level));
}
