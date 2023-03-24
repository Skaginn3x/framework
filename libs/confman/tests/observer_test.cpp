#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include "tfc/confman.hpp"

auto main(int, char**) -> int {
  using namespace boost::ut;

  "basic callback test"_test = []() {
    int some_value = 10;
    tfc::confman::observable<int> observed_value(10, [&](int new_value) { some_value = new_value; });
    observed_value.set(25);
    expect(some_value == 25);
  };
  "glaze conversion test"_test = []() {
    tfc::confman::observable<int> const observed_value(25);
    std::string const value_as_str = glz::write_json(observed_value);
    expect(value_as_str == "25");
  };
  "std containers"_test = []() {
    std::array<int, 3> array{ 1, 2, 3 };
    tfc::confman::observable<std::array<int, 3>> observed_array({ 1, 5, 8 }, [&](auto new_array) { array = new_array; });
    observed_array.set({ 7, 8, 9 });
    expect(array == std::array<int, 3>{ 7, 8, 9 });
  };
  "Callback on class"_test = []() {
    class test {
    public:
      void callback(int nvalue) { value = nvalue; }
      int value = 0;
    };
    test test_object;
    tfc::confman::observable<int> observable_value(1, [&](int new_value) { test_object.callback(new_value); });
    observable_value.set(9);
    expect(test_object.value == 9);
  };
}
