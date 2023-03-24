#include <tfc/confman.hpp>

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
