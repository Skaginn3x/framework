#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

#include <tfc/utils/json_schema.hpp>

namespace ut = boost::ut;
using ut::operator""_test;
using ut::fatal;
using ut::operator>>;

struct read_only_map_obj {
  std::map<int, int> map{};

  struct glaze {
    static constexpr auto value{ glz::object("map", &read_only_map_obj::map, tfc::json::schema{ .read_only = true }) };
  };
};

auto main() -> int {
  "read only propagates to map items"_test = [] {
    auto schema_string{ tfc::json::write_json_schema<read_only_map_obj>() };
    tfc::json::detail::schematic schema{};
    ut::expect((glz::read_json(schema, schema_string) == glz::error_code::none) >> fatal);
    ut::expect(schema.properties.has_value() >> fatal);
    ut::expect(schema.properties->at("map").read_only.has_value() >> fatal);
    ut::expect(schema.properties->at("map").read_only.value() >> fatal);
  };
  return EXIT_SUCCESS;
}
