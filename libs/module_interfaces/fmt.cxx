module;
#include <memory>
export module fmt;

import std;

extern "C++" {
#include <fmt/format.h>
#include <fmt/core.h>
#include <fmt/printf.h>
}

export namespace fmt {
using fmt::format;
using fmt::format_string;
using fmt::vformat;
using fmt::make_format_args;
using fmt::print;
using fmt::printf;
using fmt::fprintf;
}
