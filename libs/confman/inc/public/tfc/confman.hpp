#pragma once

#include <functional>
#include <string_view>
#include <type_traits>

#include <glaze/core/common.hpp>

#include <tfc/confman/observable.hpp>

namespace tfc::confman {

constexpr std::string_view socket_path = "/var/run/confman.sock";

template <typename storage_t>
class config {
public:
  explicit config(std::string_view key) : key_(key) {}
  config(std::string_view key, storage_t&& def) : key_(key), storage_(std::forward<decltype(def)>(def)) {}

  [[nodiscard]] auto get() const noexcept -> storage_t const& { return storage_; }
  void set(storage_t&& storage) { storage_ = std::forward<decltype(storage)>(storage); }

private:
  std::string key_{};
  storage_t storage_{};
};

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
