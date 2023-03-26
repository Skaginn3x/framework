#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <tfc/confman/observable.hpp>

auto main(int, char**) -> int {
  using namespace boost::ut;

  "basic callback test"_test = []() {
    int some_value = 10;
    tfc::confman::observable<int> observed_value(10, [&](int new_value, int) { some_value = new_value; });
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
    tfc::confman::observable<std::array<int, 3>> observed_array({ 1, 5, 8 },
                                                                [&](auto new_array, auto) { array = new_array; });
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
    tfc::confman::observable<int> observable_value(1, [&](int new_value, int) { test_object.callback(new_value); });
    observable_value.set(9);
    expect(test_object.value == 9);
  };

  using int_observable = tfc::confman::observable<int>;

  "copy assignment"_test = [] {
    bool called{};
    int_observable the_question{ 42, [&called](int, int) { called = true; } };
    int_observable const same_value{ the_question };
    expect(!called);
    the_question = same_value;
    expect(!called);
    int_observable const new_value{ 1337 };
    the_question = new_value;
    expect(called);
  };

  "move assignment"_test = [] {
    bool called{};
    int_observable the_question{ 42, [&called](int, int) { called = true; } };
    the_question = int_observable{ 42 };
    expect(!called);
    the_question = int_observable{ 1337 };
    expect(called);
  };
}
