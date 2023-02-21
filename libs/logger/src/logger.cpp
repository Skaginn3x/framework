#include "tfc/logger.hpp"

#include <string>
#include <spdlog/spdlog.h>

inline constexpr std::string_view LOGGING_PATTERN = "[%H:%M:%S %z] (thread %t) [%key] ";

tfc::logger::logger::logger(std::string_view key){
  
}
template <tfc::logger::lvl T, typename... P> void
tfc::logger::logger::log(std::string_view msg, P&& ...parameters) noexcept {
  spdlog::info(msg, parameters...);
}
