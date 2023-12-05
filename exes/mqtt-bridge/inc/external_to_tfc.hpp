#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <boost/asio.hpp>

#include <config/writeable_signals.hpp>
#include <config/writeable_signals_mock.hpp>
#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/logger.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

template <class ipc_client_t, class config_t, class signal_v>
class external_to_tfc {
public:
  explicit external_to_tfc(asio::io_context& io_ctx);

  auto create_outward_signals() -> void;

  auto receive_new_value(std::string signal_name, std::variant<bool, double, std::string, int64_t, uint64_t> value) -> void;

  auto last_word(std::string const& word) const -> std::string;

private:
  asio::io_context& io_ctx_;
  ipc_client_t ipc_client_{ io_ctx_ };
  tfc::logger::logger logger_{ "external_to_tfc" };
  config_t config_{ io_ctx_, "writeable_signals_config" };
  std::map<std::string, signal_v> outward_signals_;

  friend class test_external_to_tfc;
};

using ext_to_tfc = external_to_tfc<tfc::ipc_ruler::ipc_manager_client,
                                   tfc::confman::config<config::writeable_signals>,
                                   tfc::ipc::any_signal>;

extern template class external_to_tfc<tfc::ipc_ruler::ipc_manager_client,
                                      tfc::confman::config<config::writeable_signals>,
                                      tfc::ipc::any_signal>;

using any_signal_imc_mock =
    std::variant<std::monostate,                                                                              //
                 tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&>,    //
                 tfc::ipc::signal<tfc::ipc::details::type_int, tfc::ipc_ruler::ipc_manager_client_mock&>,     //
                 tfc::ipc::signal<tfc::ipc::details::type_uint, tfc::ipc_ruler::ipc_manager_client_mock&>,    //
                 tfc::ipc::signal<tfc::ipc::details::type_double, tfc::ipc_ruler::ipc_manager_client_mock&>,  //
                 tfc::ipc::signal<tfc::ipc::details::type_string, tfc::ipc_ruler::ipc_manager_client_mock&>,  //
                 tfc::ipc::signal<tfc::ipc::details::type_json, tfc::ipc_ruler::ipc_manager_client_mock&>,
                 tfc::ipc::signal<tfc::ipc::details::type_mass, tfc::ipc_ruler::ipc_manager_client_mock&> >;

extern template class external_to_tfc<tfc::ipc_ruler::ipc_manager_client_mock,
                                      tfc::mqtt::config::writeable_signals_mock,
                                      any_signal_imc_mock>;

}  // namespace tfc::mqtt
