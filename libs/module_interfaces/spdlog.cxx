module;
#include <memory>
export module spdlog;

import std;

extern "C++" {
#include <spdlog/logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/common.h>
#include <spdlog/details/console_globals.h>
#include <spdlog/details/log_msg.h>
}

export namespace spdlog {
using spdlog::async_logger;
using spdlog::sinks::base_sink;
using spdlog::details::thread_pool;
using spdlog::level_t;
using spdlog::level::level_enum;
using spdlog::string_view_t;
using spdlog::memory_buf_t;
using spdlog::details::log_msg;
using spdlog::details::null_mutex;
using spdlog::synchronous_factory;
using spdlog::throw_spdlog_ex;
}
