#include <string>

#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>
#include <tfc/utils/pragmas.hpp>
#include "custom_sink.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

inline constexpr std::string_view logging_pattern = "*** %l [%H:%M:%S %z] (thread %t) {0}.%n *** \t\t %v ";
inline constexpr size_t tp_queue_size = 128;
inline constexpr size_t tp_worker_count = 1;

namespace {
struct logger_singleton {
  logger_singleton() {
    try {
      systemd = std::make_shared<spdlog::sinks::tfc_systemd_sink_mt>();
    } catch (boost::system::system_error const& err) {
      auto loc = std::source_location::current();
      fmt::println(
          stderr,
          "Unable to open journald socket for logging. using console err: {}, source location: FILE: {}, FUNC: {}, LINE: {}",
          err.what(), loc.file_name(), loc.function_name(), loc.line());
      systemd = nullptr;
    }

    if (systemd != nullptr) {
      sinks.emplace_back(systemd);
    }

    if (tfc::base::is_stdout_enabled() || systemd == nullptr) {
      auto stdout_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

      // customize formatting for stdout messages
      stdout_sink->set_pattern(fmt::format(logging_pattern, tfc::base::get_proc_name()));
      sinks.emplace_back(stdout_sink);
    }
  }
  std::shared_ptr<spdlog::details::thread_pool> thread_pool{
    std::make_shared<spdlog::details::thread_pool>(tp_queue_size, tp_worker_count)
  };
  std::shared_ptr<spdlog::sinks::tfc_systemd_sink_mt> systemd;
  std::vector<spdlog::sink_ptr> sinks{};

  static auto instance() -> logger_singleton& {
    // clang-format off
    PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
    // clang-format on
    static logger_singleton logger_singleton_v;
    PRAGMA_CLANG_WARNING_POP
    return logger_singleton_v;
  }
};

}  // namespace

tfc::logger::logger::logger(std::string_view key) : key_{ key } {
  auto& sinks{ logger_singleton::instance().sinks };
  auto& thread_pool{ logger_singleton::instance().thread_pool };
  async_logger_ = std::make_shared<spdlog::async_logger>(key_, sinks.begin(), sinks.end(), thread_pool,
                                                         spdlog::async_overflow_policy::overrun_oldest);
  set_loglevel(tfc::base::get_log_lvl());
}
void tfc::logger::logger::log_(lvl_e log_lvl, std::string_view msg, std::source_location loc) const {
  auto const now{ spdlog::log_clock::time_point::clock::now() };
  async_logger_->log(now, spdlog::source_loc{ loc.file_name(), static_cast<int>(loc.line()), loc.function_name() },
                     static_cast<spdlog::level::level_enum>(log_lvl), msg);
}
void tfc::logger::logger::set_loglevel(tfc::logger::lvl_e log_level) {
  async_logger_->set_level(static_cast<spdlog::level::level_enum>(log_level));
}
