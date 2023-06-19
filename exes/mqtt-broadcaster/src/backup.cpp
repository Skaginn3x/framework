tfc::base::init(argc, argv);

boost::asio::io_context ctx;


std::vector<std::string> banned_strings = { "mainn", "ec_run", "operation-mode", "uint64_t" };

auto take_out_bad_strings(std::vector<std::string> signals) -> std::vector<std::string> {
  for (auto it = signals.begin(); it != signals.end();) {
    bool found = false;
    for (auto const& banned_string : banned_strings) {
      if (it->find(banned_string) != std::string::npos) {
        found = true;
        break;
      }
    }
    if (found) {
      it = signals.erase(it);
    } else {
      ++it;
    }
  }

  return signals;
}




tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);

std::vector<std::string> signals_on_client;

ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
  for (auto const& signal : signals) {
    signals_on_client.push_back(signal.name);
  }
});

ctx.run_for(std::chrono::seconds(1));

// take out all strings that are banned
signals_on_client = take_out_bad_strings(signals_on_client);

for (auto const& signal : signals_on_client) {
  std::cout << "name: " << signal << std::endl;
}

std::vector<tfc::ipc::details::any_recv_cb> connect_slots;



for (auto& signal_connect : signals_on_client) {
connect_slots.emplace_back([&ctx, &amep](std::string_view sig) -> tfc::ipc::details::any_recv_cb {
std::string slot_name = fmt::format("tfcctl_slot_{}", sig);

auto ipc{ tfc::ipc::details::create_ipc_recv_cb<tfc::ipc::details::any_recv_cb>(ctx, slot_name) };
  std::visit(

      [&](auto&& receiver) {
using receiver_t = std::remove_cvref_t<decltype(receiver)>;


if constexpr (!std::same_as<std::monostate, receiver_t>) {
          auto error = receiver->init(

              sig, [&, sig, slot_name](auto const& val) {










------------------------------------------------------------------------------------------------------------------------



#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/progbase.hpp>

          std::vector<std::string> banned_strings = { "mainn", "ec_run", "operation-mode", "uint64_t" };

          auto take_out_bad_strings(std::vector<std::string> signals) -> std::vector<std::string> {
            for (auto it = signals.begin(); it != signals.end();) {
              bool found = false;
              for (auto const& banned_string : banned_strings) {
                if (it->find(banned_string) != std::string::npos) {
                  found = true;
                  break;
                }
              }
              if (found) {
                it = signals.erase(it);
              } else {
                ++it;
              }
            }

            return signals;
          }

          auto available_signals(boost::asio::io_context& ctx, int argc, char* argv[]) -> std::vector<std::string> {
            tfc::base::init(argc, argv);

            tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);

            std::vector<std::string> signals_on_client;

            ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
              for (auto const& signal : signals) {
                signals_on_client.push_back(signal.name);
              }
            });

            signals_on_client = take_out_bad_strings(signals_on_client);

            return signals_on_client;
          }

          //  std::vector<tfc::ipc::details::any_recv_cb> connect_slots;
          //
          //  for (auto& signal_connect : signals_on_client) {
          // connect_slots.emplace_back([&ctx](std::string_view sig) -> tfc::ipc::details::any_recv_cb {
          //      std::string slot_name = fmt::format("tfcctl_slot_{}", sig);
          //
          //      auto ipc{ tfc::ipc::details::create_ipc_recv_cb<tfc::ipc::details::any_recv_cb>(ctx, slot_name) };
          //  std::visit(
          //
          //      [&](auto&& receiver) {
          //        using receiver_t = std::remove_cvref_t<decltype(receiver)>;
          //
          //        if constexpr (!std::same_as<std::monostate, receiver_t>) {
          //          auto error = receiver->init(
          //
          //              sig, [&, sig, slot_name](auto const& val) {






---------------------------------------------------------------------------------------------------


#include <tfc/confman/file_storage.hpp>
#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_server_iface.hpp>
#include <tfc/progbase.hpp>

          std::vector<std::string> banned_strings = { "mainn", "ec_run", "operation-mode", "uint64_t" };

          auto take_out_bad_strings(std::vector<std::string> signals) -> std::vector<std::string> {
            for (auto it = signals.begin(); it != signals.end();) {
              bool found = false;
              for (auto const& banned_string : banned_strings) {
                if (it->find(banned_string) != std::string::npos) {
                  found = true;
                  break;
                }
              }
              if (found) {
                it = signals.erase(it);
              } else {
                ++it;
              }
            }

            return signals;
          }

          auto available_signals(boost::asio::io_context& ctx, int argc, char* argv[]) -> std::vector<std::string> {
            tfc::base::init(argc, argv);

            tfc::ipc_ruler::ipc_manager_client ipc_client(ctx);

            std::vector<std::string> signals_on_client;

            ipc_client.signals([&](const std::vector<tfc::ipc_ruler::signal>& signals) {
              for (auto const& signal : signals) {
                signals_on_client.push_back(signal.name);
              }
            });

            signals_on_client = take_out_bad_strings(signals_on_client);

            return signals_on_client;
          }

        //  std::vector<tfc::ipc::details::any_recv_cb> connect_slots;
        //
        //  for (auto& signal_connect : signals_on_client) {
        // connect_slots.emplace_back([&ctx](std::string_view sig) -> tfc::ipc::details::any_recv_cb {
        //      std::string slot_name = fmt::format("tfcctl_slot_{}", sig);
        //
        //      auto ipc{ tfc::ipc::details::create_ipc_recv_cb<tfc::ipc::details::any_recv_cb>(ctx, slot_name) };
        //  std::visit(
        //
        //      [&](auto&& receiver) {
        //        using receiver_t = std::remove_cvref_t<decltype(receiver)>;
        //
        //        if constexpr (!std::same_as<std::monostate, receiver_t>) {
        //          auto error = receiver->init(
        //
        //              sig, [&, sig, slot_name](auto const& val) {