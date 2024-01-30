#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <boost/asio.hpp>

#include <config/bridge.hpp>
#include <config/bridge_mock.hpp>
#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/logger.hpp>

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
class external_to_tfc {
public:
  explicit external_to_tfc(asio::io_context& io_ctx, config_t& config)
      : io_ctx_(io_ctx), config_(config), ipc_client_(io_ctx) {}

  explicit external_to_tfc(asio::io_context& io_ctx, config_t& config, ipc_client_t ipc_client)
      : io_ctx_(io_ctx), config_(config), ipc_client_(ipc_client) {
    static_assert(std::is_lvalue_reference<ipc_client_t>::value);
  }

  auto create_outward_signals() -> void {
    for (auto const& sig : config_.value().writeable_signals) {
      if (!sig.name.empty()) {
        outward_signals_.emplace(sig.name, ipc::make_any<signal_v, ipc_client_t, ipc::signal>::make(
                                               sig.type, io_ctx_, ipc_client_, sig.name, sig.description));
        // no viable conversion from returned value of type
        // 'signal<details::type_bool, ipc_manager_client_mock &>'
        // (aka 'signal<type_description<bool, type_e::_bool>, tfc::ipc_ruler::ipc_manager_client_mock &>')
        // to function return type
        // 'std::variant<std::monostate, tfc::ipc::signal<tfc::ipc::details::type_description<bool, tfc::ipc::details::type_e::_bool>>, tfc::ipc::signal<tfc::ipc::details::type_description<long, tfc::ipc::details::type_e::_int64_t>>, tfc::ipc::signal<tfc::ipc::details::type_description<unsigned long, tfc::ipc::details::type_e::_uint64_t>>, tfc::ipc::signal<tfc::ipc::details::type_description<double, tfc::ipc::details::type_e::_double_t>>, tfc::ipc::signal<tfc::ipc::details::type_description<std::string, tfc::ipc::details::type_e::_string>>, tfc::ipc::signal<tfc::ipc::details::type_description<std::string, tfc::ipc::details::type_e::_json>>, tfc::ipc::signal<tfc::ipc::details::type_description<std::expected<mp_units::quantity<milli_<struct gram{{}}>{{{}}}, long>, tfc::ipc::details::mass_error_e>, tfc::ipc::details::type_e::_mass>>>'

      }
    }
  }

  auto receive_new_value(std::string signal_name, std::variant<bool, double, std::string, int64_t, uint64_t> value) -> void {
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

  auto last_word(std::string const& word) const -> std::string {
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

private:
  asio::io_context& io_ctx_;
config_t& config_;
ipc_client_t ipc_client_;
  logger::logger logger_{ "external_to_tfc" };
  std::map<std::string, signal_v> outward_signals_;

  friend class test_external_to_tfc;
};

// extern template class external_to_tfc<ipc_ruler::ipc_manager_client, confman::config<config::bridge>, ipc::any_signal>;
// extern template class external_to_tfc<ipc_ruler::ipc_manager_client, config::bridge_mock, ipc::any_signal>;
// extern template class external_to_tfc<ipc_ruler::ipc_manager_client_mock&, config::bridge_mock, ipc::any_signal>;

}  // namespace tfc::mqtt
