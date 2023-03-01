#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <iostream>
#include "tfc/confman.hpp"

auto main(int argc, char **argv) -> int {
  using namespace boost::ut;

  "basic callback test"_test = []() {
    int k = 10;
    tfc::confman::on_change<int> a(10, [&](int v){ k = v; });
    a.set(25);
    expect(k == 25);
  };
  "glaze conversion test"_test = []() {
    int k = 25;
    tfc::confman::on_change<int> b(25, [&](int v) { k = v;});
    b.set(23);
    expect(k == 23);
  };
  "std containers"_test = [](){
    std::array<int, 3> a {1,2,3} ;
    tfc::confman::on_change<std::array<int, 3>>c({1, 5, 8}, [&](auto v) { a = v; });
    c.set({7,8,9});
    expect(a == std::array<int, 3>{7,8,9});
  };
  "Callback on class"_test = [](){
    class test {
    public:
      void callback(int nvalue) {value = nvalue; }
      int value = 0;
    };
    test a;
    tfc::confman::on_change<int>c(1, [&](int v){a.callback(v); });
    c.set(9);
    expect(a.value == 9);
  };
  "Test registering standard array"_test = [](){
    std::array<int, 4> a1{1,2,3,4};
    tfc::confman::confman<std::array<int, 4>> b1;
    b1.register_service(a1);
  };
}
