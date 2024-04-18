

#include <tfc/ipc/enums.hpp>

using tfc::ipc::details::enum_cast;
using tfc::ipc::details::enum_name;
using tfc::ipc::details::type_e;

static_assert(enum_cast("blabb") == type_e::unknown);
static_assert(enum_cast("unknown") == type_e::unknown);
static_assert(enum_cast("bool") == type_e::_bool);
static_assert(enum_cast("int64_t") == type_e::_int64_t);
static_assert(enum_cast("uint64_t") == type_e::_uint64_t);
static_assert(enum_cast("double") == type_e::_double_t);
static_assert(enum_cast("string") == type_e::_string);
static_assert(enum_cast("json") == type_e::_json);
static_assert(enum_cast("mass") == type_e::_mass);
static_assert(enum_cast("length") == type_e::_length);
static_assert(enum_cast("pressure") == type_e::_pressure);
static_assert(enum_cast("temperature") == type_e::_temperature);

static_assert(enum_name(type_e::unknown) == "unknown");
static_assert(enum_name(type_e::_bool) == "bool");
static_assert(enum_name(type_e::_int64_t) == "int64_t");
static_assert(enum_name(type_e::_uint64_t) == "uint64_t");
static_assert(enum_name(type_e::_double_t) == "double");
static_assert(enum_name(type_e::_string) == "string");
static_assert(enum_name(type_e::_json) == "json");
static_assert(enum_name(type_e::_mass) == "mass");
static_assert(enum_name(type_e::_length) == "length");
static_assert(enum_name(type_e::_pressure) == "pressure");
static_assert(enum_name(type_e::_temperature) == "temperature");

auto main() -> int {
  return 0;
}
