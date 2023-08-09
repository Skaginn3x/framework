#include <tfc/ipc/item.hpp>

#include <fmt/core.h>
#include <boost/ut.hpp>

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  using boost::ut::operator>>;
  using boost::ut::fatal;

  namespace fao = tfc::ipc::item::fao;

  "test"_test = [](){
    static constexpr auto foo = fao::species::from_int(fao::atlantic_cod.to_int());
    fmt::print("foo is: {}\n", foo.code.view());

  };

  return 0;
}
