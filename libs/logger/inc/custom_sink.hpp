#pragma once

// Copyright(c) 2019 ZVYAGIN.Alexander@gmail.com
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "journald_encoding.hpp"
import asio;
import std;
import spdlog;
import tfc.base;

namespace spdlog {
namespace sinks {

using std::string_view_literals::operator""sv;
static constexpr std::string_view journald_socket = "/run/systemd/journal/socket"sv;

enum syslog_priorities : std::uint8_t {
  EMERG = 0,   /* system is unusable */
  ALERT = 1,   /* action must be taken immediately */
  CRIT = 2,    /* critical conditions */
  ERR = 3,     /* error conditions */
  WARNING = 4, /* warning conditions */
  NOTICE = 5,  /* normal but significant condition */
  INFO = 6,    /* informational */
  DEBUG = 7,   /* debug-level messages */
};

/**
 * Sink that write to systemd journal using the `sd_journal_send()` library call.
 */
template <typename mutex>
class tfc_systemd_sink : public base_sink<mutex> {
public:
  explicit tfc_systemd_sink(std::string key, bool enable_formatting = false)
      : key_{ std::move(key) }, enable_formatting_{ enable_formatting },
        syslog_levels_{
          { /* spdlog::level::trace      */ syslog_priorities::DEBUG,
            /* spdlog::level::debug      */ syslog_priorities::DEBUG,
            /* spdlog::level::info       */ syslog_priorities::INFO,
            /* spdlog::level::warn       */ syslog_priorities::WARNING,
            /* spdlog::level::err        */ syslog_priorities::ERR,
            /* spdlog::level::critical   */ syslog_priorities::CRIT,
            /* spdlog::level::off        */ syslog_priorities::INFO },
        },
        sock_{ ctx_ } {
    // This throws if the socket is not available.
    sock_.connect(journald_socket);
  }

  ~tfc_systemd_sink() override = default;

  tfc_systemd_sink(const tfc_systemd_sink&) = delete;
  auto operator=(const tfc_systemd_sink&) -> tfc_systemd_sink& = delete;

protected:
  const std::string key_;
  bool enable_formatting_ = false;
  using levels_array = std::array<int, 7>;
  levels_array syslog_levels_;
  asio::io_context ctx_;
  asio::local::datagram_protocol::socket sock_;

  void sink_it_(const details::log_msg& msg) override {
    string_view_t payload;
    memory_buf_t formatted;
    if (enable_formatting_) {
      base_sink<mutex>::formatter_->format(msg, formatted);
      payload = string_view_t(formatted.data(), formatted.size());
    } else {
      payload = msg.payload;
    }

    std::size_t length = payload.size();
    // limit to max int
    if (length > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
      length = static_cast<std::size_t>(std::numeric_limits<int>::max());
    }

    std::vector<std::pair<std::string_view, std::string_view>> parameters;
    parameters.emplace_back("TFC_KEY", key_);
    parameters.emplace_back("TFC_EXE", tfc::base::get_exe_name());
    parameters.emplace_back("TFC_ID", tfc::base::get_proc_name());

    // Container for holding the lifetime for the string of the source line
    std::string src_line;
    if (!msg.source.empty()) {
      parameters.emplace_back("CODE_FILE", msg.source.filename);
      src_line = std::to_string(msg.source.line);
      parameters.emplace_back("CODE_LINE", src_line);
      parameters.emplace_back("CODE_FUNC", msg.source.funcname);
    }

    parameters.emplace_back("MESSAGE", payload);

    auto to_transmit = tfc::logger::journald::to_message(parameters);

    try {
      sock_.send(asio::buffer(to_transmit));
    } catch (std::system_error const& error) {
      throw_spdlog_ex(fmt::format("Failed writing to systemd {}", error.what()));
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
