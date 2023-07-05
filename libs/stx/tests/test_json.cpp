
#include <tfc/utils/json.hpp>

using tfc::json::schema_attributes;

struct test {
  int a{};
  double b{};
  struct glaze {
    static auto constexpr value{  tfc::json::object(
        "a", &test::a, schema_attributes{ .description="length to sensor",.min=0,.max=300,.default_value=std::uint64_t{20} },
        "b", &test::b, "b comment"
        ) };
  };
};

int main() {
  test instance{};
  auto json{ glz::write_json(instance) };
  [[maybe_unused]] test::glaze const is_this_possible{};
  [[maybe_unused]] auto const value{  tfc::json::object(
        "a", &test::a, schema_attributes{ .description="length to sensor",.min=0,.max=300,.default_value=std::uint64_t{20} },
        "b", &test::b, "b comment"
    ) };
  return 0;
}
