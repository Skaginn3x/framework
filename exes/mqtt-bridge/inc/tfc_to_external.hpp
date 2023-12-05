#pragma once

#include <chrono>
#include <concepts>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/asio.hpp>
#include <sdbusplus/bus/match.hpp>

#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>

#include <spark_plug_interface.hpp>
#include <structs.hpp>

namespace tfc::mqtt {

namespace asio = boost::asio;

template <class spark_plug_config_t, class mqtt_client_t, class ipc_client_t>
class tfc_to_external {
public:
  explicit tfc_to_external(asio::io_context& io_ctx,
                           tfc::mqtt::spark_plug_interface<spark_plug_config_t, mqtt_client_t>& spark_plug_i)
      : io_ctx_(io_ctx), spark_plug_interface_(spark_plug_i), ipc_client_(io_ctx_) {}

  explicit tfc_to_external(asio::io_context& io_ctx,
                           tfc::mqtt::spark_plug_interface<spark_plug_config_t, mqtt_client_t>& spark_plug_i,
                           ipc_client_t ipc_client)
      : io_ctx_(io_ctx), spark_plug_interface_(spark_plug_i), ipc_client_(ipc_client) {
    static_assert(std::is_lvalue_reference<ipc_client_t>::value);
  }

  /// This function converts tfc types to Spark Plug B types
  /// More information can be found (page 76) in the spec under section 6.4.16 data types:
  /// https://sparkplug.eclipse.org/specification/version/3.0/documents/sparkplug-specification-3.0.0.pdf
  static auto type_enum_convert(tfc::ipc::details::type_e type) -> DataType {
    using enum tfc::ipc::details::type_e;
    switch (type) {
      case unknown:
        return DataType::Unknown;
      case _bool:
        return DataType::Boolean;
      case _int64_t:
        return DataType::Int64;
      case _uint64_t:
        return DataType::UInt64;
      case _double_t:
        return DataType::Double;
      case _string:
      case _json:
        return DataType::String;
    }
    return DataType::Unknown;
  }

  /// Spark Plug B disallows the use of some special characters in the signal name.
  static auto format_signal_name(std::string signal_name_to_format) -> std::string {
    for (const auto& forbidden_char : { '+', '#', '-', '/' }) {
      std::ranges::replace(signal_name_to_format.begin(), signal_name_to_format.end(), forbidden_char, '_');
    }

    std::ranges::replace(signal_name_to_format.begin(), signal_name_to_format.end(), '.', '/');
    return signal_name_to_format;
  }

  auto set_signals() -> void {
    logger_.trace("Starting to add new signals...");
    ipc_client_.signals(
        [this](std::vector<ipc_ruler::signal> const& signals) { handle_incoming_signals_from_ipc_client(signals); });
  }

  auto handle_incoming_signals_from_ipc_client(std::vector<tfc::ipc_ruler::signal> const& signals) -> void {
    logger_.trace("Received {} new signals to add.", signals.size());

    signals_.clear();
    spb_variables_.clear();

    signals_.reserve(signals.size());
    spb_variables_.reserve(signals.size());

    for (const auto& signal : signals) {
      signals_.emplace_back(ipc::details::make_any_slot_cb::make(signal.type, io_ctx_, signal.name));

      spb_variables_.emplace_back(format_signal_name(signal.name.data()),
                                  type_enum_convert(signal.type), std::nullopt, signal.description.data());

      logger_.trace("Connecting: {}", signal.name);

      auto& slot = signals_.back();
      auto& variable = spb_variables_.back();

      std::visit(
          [this, &variable](auto&& receiver) {
            using receiver_t = std::remove_cvref_t<decltype(receiver)>;
            if constexpr (!std::same_as<receiver_t, std::monostate>) {
              auto error_code = receiver->connect(receiver->name(), [this, &variable](auto&& value) {
                variable.value = value;
                spark_plug_interface_.update_value(variable);
              });
              if (error_code) {
                logger_.trace("Error connecting to signal: {}, error: {}", receiver->name(), error_code.message());
              }
            }
          },
          slot);
    }

    logger_.trace("All new signals added. Preparing to send NBIRTH and start signals...");

    set_current_values();
  }

  auto set_current_values() -> void {
    logger_.info("Setting current values to interface");

    spark_plug_interface_.set_current_values(spb_variables_);
    spark_plug_interface_.send_current_values();
  }

  auto get_signals() -> std::vector<tfc::ipc::details::any_slot_cb>& { return signals_; }

  auto clear_signals() -> void { signals_.clear(); }

private:
  asio::io_context& io_ctx_;
  spark_plug_interface<spark_plug_config_t, mqtt_client_t>& spark_plug_interface_;
  ipc_client_t ipc_client_;
  tfc::logger::logger logger_{ "tfc_to_external" };
  std::vector<tfc::ipc::details::any_slot_cb> signals_;
  std::vector<spark_plug_b_variable> spb_variables_;

  friend class test_tfc_to_external;
};

using tfc_to_ext = tfc_to_external<tfc::confman::config<config::spark_plug_b>,
                                   client<endpoint_client, tfc::confman::config<config::broker>>,
                                   tfc::ipc_ruler::ipc_manager_client>;

using tfc_to_ext_mock = tfc_to_external<config::spark_plug_b_mock,
                                        client<endpoint_client_mock, config::broker_mock>,
                                        tfc::ipc_ruler::ipc_manager_client_mock&>;

}  // namespace tfc::mqtt
