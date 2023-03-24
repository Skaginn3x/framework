#include <boost/ut.hpp>

#include <tfc/confman.hpp>

namespace ut = boost::ut;
using ut::operator""_test;

namespace tfc::confman {

struct storage_example {
  int a{};
  double b{};
  observable<bool> active{};
  struct glaze {
    using T = storage_example;
    static constexpr auto value = glz::object("a", &T::a, "b", &T::b, "active", &T::active);
  };
};

class use_case {
public:
  use_case() {
    config_.get().active.observe([](bool new_value) {
      // do something
    });
  }

private:
  config<storage_example> config_{ "key.1", storage_example{ .a = 1 } };
};

}  // namespace tfc::confman

auto main(int, char**) -> int {
  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
    "happy confman"_test = [] {

    };
  };
  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
