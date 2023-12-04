#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <fmt/core.h>
#include <mp-units/format.h>
#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;
namespace ipc = tfc::ipc;

inline auto stdin_coro(asio::io_context& ctx, std::string_view signal_name) -> asio::awaitable<void> {
  auto executor = co_await asio::this_coro::executor;
  asio::posix::stream_descriptor input_stream(executor, STDIN_FILENO);

  auto client{ tfc::ipc::make_manager_client(ctx) };

  auto type{ ipc::details::enum_cast(signal_name) };
  if (type == ipc::details::type_e::unknown) {
    throw std::runtime_error{ fmt::format("Unknown typename in: {}\n", signal_name) };
  }
  auto sender{ ipc::make_any_signal::make(type, ctx, client, signal_name) };

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
            if constexpr (signal_type::value_type == ipc::details::type_e::_string) {
              buff = fmt::format("\"{}\"", buffer_str);
            }
            auto value{ glz::read_json<value_t>(buff) };
            if (!value.has_value()) {
              fmt::print("Invalid input error: {}\n", glz::format_error(value.error(), buff));
              return;
            }

            my_signal.async_send(value.value(), [&, actual_value = value.value()](std::error_code const& code, size_t bytes) {
              if (code) {
                fmt::print("Error: {}\n", code.message());
              } else {
                if constexpr (tfc::stx::is_expected_quantity<value_t>) {
                  fmt::print("Sent value: {} size: {}\n", actual_value.value(), bytes);
                }
                else {
                  fmt::print("Sent value: {} size: {}\n", actual_value, bytes);
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
  std::string slot{};
  std::vector<std::string> connect;
  bool list_signals{};
  bool list_slots{};

  description.add_options()("signal", po::value<std::string>(&signal), "IPC signal channel (output)")(
      "slot", po::value<std::string>(&slot), "IPC slot channel (input)")(
      "connect,c", po::value<std::vector<std::string>>(&connect)->multitoken(), "Listen to these slots")(
      "list-signals", po::bool_switch(&list_signals), "List all available IPC signals")(
      "list-slots", po::bool_switch(&list_slots), "List all available IPC slots");
  tfc::base::init(argc, argv, description);

  // Must provide an argument
  if (tfc::base::get_map().find("signal") == tfc::base::get_map().end() && connect.empty()) {
    std::stringstream out;
    description.print(out);
    fmt::print("Usage: tfcctl [options] \n{}\n", out.str());
    std::exit(0);
  }

  asio::io_context ctx;

  if (list_signals) {
    auto client{ tfc::ipc::make_manager_client(ctx) };
    client.signals([&logger](std::vector<tfc::ipc_ruler::signal> const& signals) {
      for (const auto& sig : signals) {
        logger.trace("{}", sig.name);
      }
    });
    ctx.run_for(std::chrono::milliseconds(100));
    return 0;
  }

  if (list_slots) {
    auto client{ tfc::ipc::make_manager_client(ctx) };
    client.slots([&logger](std::vector<tfc::ipc_ruler::slot> const& slots) {
      for (const auto& sl : slots) {
        logger.trace("{}", sl.name);
      }
    });
    ctx.run_for(std::chrono::milliseconds(100));
    return 0;
  }

  // For sending a signal
  if (!signal.empty()) {
    asio::co_spawn(ctx, stdin_coro(ctx, signal), asio::detached);
  }

  std::vector<tfc::ipc::details::any_slot_cb> connect_slots;

  auto constexpr slot_connect{ [](auto&& receiver_variant, std::string_view signal_name) {
    std::visit(
        [signal_name]<typename receiver_t>(receiver_t&& receiver) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<receiver_t>>) {
            fmt::print("Connecting to signal {}\n", signal_name);
            std::string sig{ signal_name };
            auto error = receiver->connect(signal_name, [sig]<typename val_t>(val_t const& val) {
              if constexpr (tfc::stx::is_specialization_v<std::remove_cvref_t<val_t>, std::expected>) {
                if (val.has_value()) {
                  fmt::print("{}: {}\n", sig, val.value());
                } else {
                  fmt::print("{}: {}\n", sig, val.error());
                }
              } else {
                fmt::print("{}: {}\n", sig, val);
              }
            });
            if (error) {
              fmt::print("Failed to connect: {}\n", error.message());
            }
          }
        },
        receiver_variant);
  } };

  for (auto& signal_connect : connect) {
    // For listening to connections
    connect_slots.emplace_back([&ctx, slot_connect](std::string_view sig) -> tfc::ipc::details::any_slot_cb {
      std::string const slot_name = fmt::format("tfcctl_slot_{}", sig);
      auto const type{ ipc::details::enum_cast(sig) };
      if (type == ipc::details::type_e::unknown) {
        throw std::runtime_error{ fmt::format("Unknown typename in: {}\n", sig) };
      }
      auto ipc{ ipc::details::make_any_slot_cb::make(type, ctx, slot_name) };
      slot_connect(ipc, sig);
      return ipc;
    }(signal_connect));
  }

  asio::signal_set signal_set{ ctx, SIGINT, SIGTERM, SIGHUP };
  signal_set.async_wait([&](const std::error_code& error, int signal_number) {
    if (error) {
      fmt::print(stderr, "Error waiting for signal. {}\n", error.message());
    }
    fmt::print("Shutting down signal ({})\n", signal_number);
    ctx.stop();
  });

  ctx.run();
  return 0;
}
