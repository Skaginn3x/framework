#pragma once

#include <cstdint>
#include <map>
#include <ranges>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/asio.hpp>

#include <tfc/ipc.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

template <class ipc_client_t, class config_t>
class external_to_tfc {
public:
  explicit external_to_tfc(asio::io_context& io_ctx, config_t& config, ipc_client_t ipc_client)
      : io_ctx_(io_ctx), config_(config), ipc_client_(ipc_client) {
    static_assert(std::is_lvalue_reference<ipc_client_t>::value);
  }

  using signal_v = std::variant<std::monostate,
                                ipc::signal<ipc::details::type_bool, ipc_client_t>,
                                ipc::signal<ipc::details::type_int, ipc_client_t>,
                                ipc::signal<ipc::details::type_uint, ipc_client_t>,
                                ipc::signal<ipc::details::type_double, ipc_client_t>,
                                ipc::signal<ipc::details::type_string, ipc_client_t>,
                                ipc::signal<ipc::details::type_json, ipc_client_t>,
                                ipc::signal<ipc::details::type_mass, ipc_client_t> >;

  auto create_outward_signals() -> void {
    for (auto const& sig : config_.value().writeable_signals) {
      if (!sig.name.empty()) {
        outward_signals_.emplace(sig.name, ipc::make_any<signal_v, ipc_client_t, ipc::signal>::make(
                                               sig.type, io_ctx_, ipc_client_, sig.name, sig.description));
      }
    }
  }

  auto receive_new_value(std::string signal_name, std::variant<bool, double, std::string, int64_t, uint64_t> value) -> void {
    logger_.trace("Received new value for signal: {}", signal_name);

    std::optional<std::string> sig_name{ last_word(signal_name) };

    if (sig_name.has_value()) {
      std::visit(
          [this, &value]<typename signal_t>(signal_t& signal) {
            if constexpr (!std::is_same_v<std::remove_cvref_t<signal_t>, std::monostate>) {
              using value_t = typename std::remove_cvref_t<signal_t>::value_t;

              // Spark Plug B sends int64 values as uint64
              // when an int64 value arrives it is in the form of uint64
              // therefore the uint64 value needs to be "get" and cast into int64
              // to the signal
              if constexpr (std::is_same_v<value_t, int64_t>) {
                signal.send(static_cast<int64_t>(std::get<uint64_t>(value)));
              } else if constexpr (tfc::stx::is_expected_quantity<value_t>) {
                logger_.trace("Mass type hasn't been implemented");
              } else {
                signal.send(std::get<value_t>(value));
              }
            }
          },
          outward_signals_.at(sig_name.value()));
    }
  }

  auto last_word(std::string const& word) const -> std::optional<std::string> {
    auto rview = std::views::reverse(word);
    auto result = std::ranges::find(rview, '/');

    if (result != rview.end()) {
      auto distance = std::distance(rview.begin(), result);
      return std::string(word.end() - distance, word.end());
    }
    return std::nullopt;
  }

private:
  asio::io_context& io_ctx_;
  config_t& config_;
  ipc_client_t ipc_client_;
  logger::logger logger_{ "external_to_tfc" };
  std::map<std::string, signal_v> outward_signals_;

  friend class test_external_to_tfc;
};

}  // namespace tfc::mqtt
