#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>

#include <boost/asio.hpp>

#include <external_to_tfc.hpp>
#include <tfc/ipc.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

template <class ipc_client_t, class config_t, class signal_v>
external_to_tfc<ipc_client_t, config_t, signal_v>::external_to_tfc(asio::io_context& io_ctx) : io_ctx_(io_ctx) {}

template <class ipc_client_t, class config_t, class signal_v>
auto external_to_tfc<ipc_client_t, config_t, signal_v>::create_outward_signals() -> void {
  for (auto const& sig : config_.value().writeable_signals) {
    if (!sig.name.empty()) {
      outward_signals_.emplace(sig.name, tfc::ipc::make_any<signal_v, ipc_client_t&, tfc::ipc::signal>::make(
                                             sig.type, io_ctx_, ipc_client_, sig.name, sig.description));
    }
  }
}

template <class ipc_client_t, class config_t, class signal_v>
auto external_to_tfc<ipc_client_t, config_t, signal_v>::receive_new_value(
    std::string signal_name,
    std::variant<bool, double, std::string, int64_t, uint64_t> value) -> void {
  logger_.trace("Received new value for signal: {}", signal_name);

  std::visit(
      [&value]<typename signal_t>(signal_t& signal) {
        if constexpr (!std::is_same_v<std::remove_cvref_t<signal_t>, std::monostate>) {
          using value_t = typename std::remove_cvref_t<signal_t>::value_t;

          // Spark Plug B sends int64 values as uint64
          // when an int64 value arrives it is in the form of uint64
          // therefore the uint64 value needs to be "get" and cast into int64
          // to the signal
          if constexpr (std::is_same_v<value_t, int64_t>) {
            signal.send(static_cast<int64_t>(std::get<uint64_t>(value)));
          } else if constexpr (tfc::stx::is_expected_quantity<value_t>) {
            // todo
          } else {
            signal.send(std::get<value_t>(value));
          }
        }
      },
      outward_signals_.at(last_word(signal_name)));
}

template <class ipc_client_t, class config_t, class signal_v>
auto external_to_tfc<ipc_client_t, config_t, signal_v>::last_word(std::string const& word) const -> std::string {
  std::stringstream test(word);
  std::string segment;
  std::vector<std::string> seglist;

  while (std::getline(test, segment, '/')) {
    seglist.push_back(segment);
  }

  if (!seglist.empty()) {
    return seglist.back();
  }
  return "";
}

// template class external_to_tfc<tfc::ipc_ruler::ipc_manager_client,
//                                tfc::confman::config<config::writeable_signals>,
//                                tfc::ipc::any_signal>;
//
// template class external_to_tfc<tfc::ipc_ruler::ipc_manager_client_mock, config::writeable_signals_mock, any_signal_imc_mock>;

}  // namespace tfc::mqtt
