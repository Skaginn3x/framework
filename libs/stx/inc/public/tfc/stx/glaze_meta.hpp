#pragma once
#include <date/date.h>
#include <glaze/glaze.hpp>
#include <sstream>

static inline auto parse8601(const std::string& save) -> date::sys_time<std::chrono::milliseconds> {
  std::istringstream in{ save };
  date::sys_time<std::chrono::milliseconds> tp;
  in >> date::parse("%FT%TZ", tp);
  if (in.fail()) {
    in.clear();
    in.exceptions(std::ios::failbit);
    in.str(save);
    in >> date::parse("%FT%T%Ez", tp);
  }
  return tp;
}

namespace glz::detail {
template <>
struct from_json<std::chrono::time_point<std::chrono::system_clock>> {
  template <auto Opts>
  static void op(auto& value, auto&&... args) {
    std::string rep;
    read<json>::op<Opts>(rep, args...);
    value = parse8601(rep);
  }
};

template <>
struct to_json<std::chrono::time_point<std::chrono::system_clock>> {
  template <auto Opts>
  static void op(auto& value, auto&&... args) noexcept {
    std::stringstream str_stream;
    str_stream << date::format("%FT%T%Ez", value);
    write<json>::op<Opts>(str_stream.str(), args...);
  }
};
}  // namespace glz::detail
