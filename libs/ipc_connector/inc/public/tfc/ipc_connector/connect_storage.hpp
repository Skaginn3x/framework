#pragma once

#include <string>

#include <glaze/core/common.hpp>

#include <tfc/confman/observable.hpp>

namespace tfc::ipc {

struct connect_storage {
  tfc::confman::observable<std::string> signal_name{};
  struct glaze {
    static constexpr auto value{ glz::object("signal_name", &connect_storage::signal_name) };
  };
};

}  // namespace tfc::ipc
