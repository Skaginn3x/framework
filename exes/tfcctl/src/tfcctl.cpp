#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <fmt/core.h>
#include <mp-units/format.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;
namespace ipc = tfc::ipc;

using namespace std::chrono_literals;

inline auto stdin_coro(asio::io_context& ctx, std::string_view signal_name) -> asio::awaitable<void> {
  auto executor = co_await asio::this_coro::executor;
  asio::posix::stream_descriptor input_stream(executor, STDIN_FILENO);

  auto client{ tfc::ipc_ruler::ipc_manager_client(ctx) };

  auto type{ ipc::details::enum_cast(signal_name) };
  if (type == ipc::details::type_e::unknown) {
    throw std::runtime_error{ fmt::format("Unknown typename in: {}", signal_name) };
  }
  auto sender{ ipc::make_any_signal::make(type, ctx, client, signal_name) };
  std::visit(
      []<typename signal_t>(signal_t& my_signal) {
        if constexpr (!std::same_as<std::remove_cvref_t<signal_t>, std::monostate>) {
          fmt::println("Registered signal with name: {}", my_signal.full_name());
        }
      },
      sender);

  while (true) {
    co_await input_stream.async_wait(asio::posix::stream_descriptor::wait_read, asio::use_awaitable);
    std::array<char, 1024> buffer;
    const std::size_t bytes_read = co_await input_stream.async_read_some(asio::buffer(buffer), asio::use_awaitable);
    std::string_view buffer_str{ std::begin(buffer), bytes_read - 1 };  // strip of the new line character

    std::visit(
        [buffer_str]<typename signal_t>(signal_t&& my_signal) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<signal_t>>) {
            using signal_type = std::remove_cvref_t<signal_t>;
            using value_t = typename signal_type::value_t;
            std::string buff{ buffer_str };
            value_t val{};
            if constexpr (signal_type::value_type == ipc::details::type_e::_string ||
                          signal_type::value_type == ipc::details::type_e::_json) {
              val = buff;
            } else {
              auto value{ glz::read_json<value_t>(buff) };
              if (!value.has_value()) {
                fmt::println("Invalid input error: {}", glz::format_error(value.error(), buff));
                return;
              }
              val = value.value();
            }

            my_signal.async_send(val, [&, actual_value = val](std::error_code const& code, size_t bytes) {
              if (code) {
                fmt::println("Error: {}", code.message());
              } else {
                if constexpr (tfc::stx::is_expected_quantity<value_t>) {
                  fmt::println("Sent value: {} size: {}", actual_value.value(), bytes);
                } else {
                  fmt::println("Sent value: {} size: {}", actual_value, bytes);
                }
              }
            });
          }
        },
        sender);
  }
}

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };

  std::string signal{};
  std::string slot_name{};
  std::vector<std::string> connect;
  bool list_signals{};
  bool list_slots{};

  description.add_options()("signal", po::value<std::string>(&signal), "IPC signal channel (output)")(
      "slot", po::value<std::string>(&slot_name), "IPC slot channel (input)")(
      "connect,c", po::value<std::vector<std::string>>(&connect)->multitoken(), "Listen to these slots")(
      "list-signals", po::bool_switch(&list_signals), "List all available IPC signals")(
      "list-slots", po::bool_switch(&list_slots), "List all available IPC slots");
  tfc::base::init(argc, argv, description);

  bool at_least_one_choice{ !signal.empty() || !slot_name.empty() || !connect.empty() || list_signals || list_slots };
  if (!at_least_one_choice) {
    std::stringstream out;
    description.print(out);
    fmt::println("Usage: tfcctl [options] \n{}", out.str());
    std::exit(0);
  }


  if (list_signals || list_slots) {
    // Creating a manager object creates a match rule that is listened to until the object is deconstructed.
    // This does not look good but is the simplest method of getting the desired effect from the code
    // As it is today
    asio::io_context list_ctx;
    tfc::ipc_ruler::ipc_manager_client manager_client(list_ctx);
    const auto print_names{[](std::string context, auto const& instances) {
      fmt::println("{}", context);
      for (const auto& instance : instances) {
        fmt::println("{}", instance.name);
      }
    } };
    if (list_signals) {
      manager_client.signals(std::bind_front(print_names, "Signals:"));
    }
    if (list_slots) {
      manager_client.slots(std::bind_front(print_names, "Slots:"));
    }
    list_ctx.run_for(100ms);
  }

  asio::io_context ctx;
  // For sending a signal
  if (!signal.empty()) {
    asio::co_spawn(ctx, stdin_coro(ctx, signal), asio::detached);
  }

  std::vector<tfc::ipc::details::any_slot_cb> connect_slots;

  auto constexpr slot_connect{ [](auto&& receiver_variant, std::string_view signal_name) {
    std::visit(
        [signal_name]<typename receiver_t>(receiver_t&& receiver) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<receiver_t>>) {
            fmt::println("Connecting to signal {}", signal_name);
            std::string sig{ signal_name };
            auto error = receiver->connect(signal_name, [sig]<typename val_t>(val_t const& val) {
              if constexpr (tfc::stx::is_expected<std::remove_cvref_t<val_t>>) {
                if (val.has_value()) {
                  fmt::println("{}: {}", sig, val.value());
                } else {
                  fmt::println("{}: {}", sig, val.error());
                }
              } else {
                fmt::println("{}: {}", sig, val);
              }
              fflush(stdout);
            });
            if (error) {
              fmt::println("Failed to connect: {}", error.message());
            }
          }
        },
        receiver_variant);
  } };

  for (auto& signal_connect : connect) {
    // For listening to connections
    connect_slots.emplace_back([&ctx, slot_connect](std::string_view sig) -> tfc::ipc::details::any_slot_cb {
      std::string const connected_slot_name = fmt::format("tfcctl_slot_{}", sig);
      auto const type{ ipc::details::enum_cast(sig) };
      if (type == ipc::details::type_e::unknown) {
        throw std::runtime_error{ fmt::format("Unknown typename in: {}", sig) };
      }
      auto ipc{ ipc::details::make_any_slot_cb::make(type, ctx, connected_slot_name) };
      slot_connect(ipc, sig);
      return ipc;
    }(signal_connect));
  }

  std::shared_ptr<tfc::ipc_ruler::ipc_manager_client> client;
  tfc::ipc::any_slot slot;

  if (!slot_name.empty()) {
    client = std::make_shared<tfc::ipc_ruler::ipc_manager_client>(ctx);
    // For listening to connections
    auto const type{ ipc::details::enum_cast(slot_name) };
    if (type == ipc::details::type_e::unknown) {
      throw std::runtime_error{ fmt::format("Unknown typename in: {}", slot_name) };
    }
    tfc::ipc::make_any_slot::make_impl(slot, type, ctx, *client.get(), slot_name, [slot_name](auto new_value) {
      if constexpr (tfc::stx::is_expected<std::remove_cvref_t<decltype(new_value)>>) {
        if (new_value.has_value()) {
          fmt::println("{}: {}", slot_name, new_value.value());
        } else {
          fmt::println("{}: {}", slot_name, new_value.error());
        }
      } else {
        fmt::println("{}: {}", slot_name, new_value);
      }
    });
  }

  ctx.run();
  return 0;
}
