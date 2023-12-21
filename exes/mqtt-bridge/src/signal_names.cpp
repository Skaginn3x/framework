#include <signal_names.hpp>

namespace {
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wexit-time-destructors)
thread_local std::vector<tfc::ipc_ruler::signal> signals{};
PRAGMA_CLANG_WARNING_POP
// clang-format on
}  // namespace
namespace tfc::global {  // please name better
void set_signals(std::vector<tfc::ipc_ruler::signal> const& names) {
  signals = names;
}
std::vector<tfc::ipc_ruler::signal> const& get_signals() {
  return signals;
}
}  // namespace tfc::global
