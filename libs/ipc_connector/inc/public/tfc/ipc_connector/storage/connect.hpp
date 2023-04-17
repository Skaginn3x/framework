#pragma once

#include <string>

#include <glaze/core/common.hpp>

#include <tfc/confman/observable.hpp>

namespace tfc::ipc::storage {

struct connect {
  tfc::confman::observable<std::string> signal_name{};
  struct glaze {
    static constexpr auto value{ glz::object("signal_name", &connect::signal_name) };
  };
};

}  // namespace tfc::ipc::storage
