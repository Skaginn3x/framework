#pragma once

#include <string>
#include <vector>

#include <tfc/ipc.hpp>

/// TODO: better name
/// brainstorm idea: signal_names.hpp
namespace tfc::global {
void set_signals(std::vector<tfc::ipc_ruler::signal> const&);
std::vector<tfc::ipc_ruler::signal> const& get_signals();
}  // namespace tfc::global
