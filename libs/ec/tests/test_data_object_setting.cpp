#include <chrono>

#include <mp-units/systems/si/si.h>

#include <tfc/ec/data_object_setting.hpp>

using tfc::ec::setting;

namespace example {
enum struct enum_value_e : int8_t { on = 1, off = 2 };
using enum_setting = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", enum_value_e, enum_value_e::on>;
using trivial_type_setting = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", enum_value_e, enum_value_e::on>;

// using std chrono
using chrono_test = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", std::chrono::milliseconds, 60>;
static_assert(sizeof(std::chrono::milliseconds) == 8);
static_assert(std::is_same_v<chrono_test::period_type, std::milli>);
static_assert(std::is_same_v<chrono_test::rep_type, std::chrono::milliseconds::rep>);

// using mp units

using mp_units::quantity;
using mp_units::si::watt;
static_assert(sizeof(quantity<watt, uint32_t>) == 4);
using mp_test = setting<ecx::index_t{ 0x42, 0x42 }, "name", "desc", quantity<watt, uint32_t>, 60 * watt>;
static_assert(std::is_same_v<mp_test::rep_type, uint32_t>);

}  // namespace example

auto main(int, char**) -> int {
  [[maybe_unused]] example::enum_setting const test1{};
  [[maybe_unused]] example::trivial_type_setting const test2{};
  [[maybe_unused]] example::chrono_test const test3{};
  [[maybe_unused]] example::mp_test const test4{};

  return EXIT_SUCCESS;
}
