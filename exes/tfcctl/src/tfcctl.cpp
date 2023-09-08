#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;
namespace ipc = tfc::ipc;

inline auto stdin_coro(asio::io_context& ctx, tfc::logger::logger& logger, std::string_view signal_name)
    -> asio::awaitable<void> {
  auto executor = co_await asio::this_coro::executor;
  asio::posix::stream_descriptor input_stream(executor, STDIN_FILENO);

  auto client{ tfc::ipc::make_manager_client(ctx) };

  auto type{ ipc::details::string_to_type(signal_name) };
  if (type == ipc::details::type_e::unknown) {
    throw std::runtime_error{ fmt::format("Unknown typename in: {}\n", signal_name) };
  }
  auto sender{ ipc::make_any_signal::make(type, ctx, client, signal_name) };

  while (true) {
    co_await input_stream.async_wait(asio::posix::stream_descriptor::wait_read, asio::use_awaitable);
    std::array<char, 1024> buffer;
    const std::size_t bytes_read = co_await input_stream.async_read_some(asio::buffer(buffer), asio::use_awaitable);
    std::string_view buffer_str{ std::begin(buffer), bytes_read - 1 };  // strip of the new line character

    constexpr auto send{ [](std::string_view input, auto& in_sender, auto& in_logger) -> void {
      try {
        using value_t = typename std::remove_reference_t<decltype(in_sender)>::value_t;
        auto value = boost::lexical_cast<value_t>(input);
        in_sender.async_send(value, [&, value](std::error_code code, size_t bytes) {
          if (code) {
            in_logger.template log<tfc::logger::lvl_e::error>("Error: {}", code.message());
          } else {
            in_logger.template log<tfc::logger::lvl_e::info>("Sent value: {} size: {}", value, bytes);
          }
        });
      } catch (boost::bad_lexical_cast const& bad_lexical_cast) {
        in_logger.template log<tfc::logger::lvl_e::info>("Invalid input {}, error: {}", input, bad_lexical_cast.what());
      }
    } };
    if (auto* bool_sender{ std::get_if<ipc::bool_signal>(&sender) }) {
      send(buffer_str, *bool_sender, logger);
    } else if (auto* int_sender{ std::get_if<ipc::int_signal>(&sender) }) {
      send(buffer_str, *int_sender, logger);
    } else if (auto* uint_sender{ std::get_if<ipc::uint_signal>(&sender) }) {
      send(buffer_str, *uint_sender, logger);
    } else if (auto* double_sender{ std::get_if<ipc::double_signal>(&sender) }) {
      send(buffer_str, *double_sender, logger);
    } else if (auto* string_sender{ std::get_if<ipc::string_signal>(&sender) }) {
      send(buffer_str, *string_sender, logger);
    } else if (auto* json_sender{ std::get_if<ipc::json_signal>(&sender) }) {
      send(buffer_str, *json_sender, logger);
    }
  }
}

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };

  std::string signal{};
  std::string slot{};

  std::vector<std::string> connect;

  description.add_options()("signal", po::value<std::string>(&signal), "IPC signal channel (output)")(
      "slot", po::value<std::string>(&slot), "IPC slot channel (input)")(
      "connect,c", po::value<std::vector<std::string>>(&connect)->multitoken(), "Listen to these slots");
  tfc::base::init(argc, argv, description);

  // Must provide an argument
  if (tfc::base::get_map().find("signal") == tfc::base::get_map().end() && connect.empty()) {
    std::stringstream out;
    description.print(out);
    fmt::print("Usage: tfcctl [options] \n{}", out.str());
    std::exit(0);
  }
  tfc::logger::logger logger{ "tfc control" };

  asio::io_context ctx;

  // For sending a signal
  if (!signal.empty()) {
    asio::co_spawn(ctx, stdin_coro(ctx, logger, signal), asio::detached);
  }

  std::vector<tfc::ipc::details::any_slot_cb> connect_slots;

  auto constexpr slot_connect{ [](auto&& receiver_variant, std::string_view signal_name, auto&& in_logger) {
    std::visit(
        [signal_name, &in_logger]<typename receiver_t>(receiver_t&& receiver) {
          if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<receiver_t>>) {
            in_logger.trace("Connecting to signal {}", signal_name);
            auto error = receiver->connect(signal_name);
            if (error) {
              in_logger.error("Failed to connect: {}", error.message());
            }
          }
        },
        receiver_variant);
  } };

  for (auto& signal_connect : connect) {
    // For listening to connections
    connect_slots.emplace_back([&ctx, &logger, slot_connect](std::string_view sig) -> tfc::ipc::details::any_slot_cb {
      std::string const slot_name = fmt::format("tfcctl_slot_{}", sig);
      auto const type{ ipc::details::string_to_type(sig) };
      if (type == ipc::details::type_e::unknown) {
        throw std::runtime_error{ fmt::format("Unknown typename in: {}\n", sig) };
      }
      auto ipc{ ipc::details::make_any_slot_cb::make(type, ctx, slot_name, [sig, &logger](auto const& val) {
        logger.info("{}: {}", sig, val);  //
      }) };
      slot_connect(ipc, sig, logger);
      return ipc;
    }(signal_connect));
  }

  asio::signal_set signal_set{ ctx, SIGINT, SIGTERM, SIGHUP };
  signal_set.async_wait([&](const std::error_code& error, int signal_number) {
    if (error) {
      logger.log<tfc::logger::lvl_e::error>("Error waiting for signal. {}", error.message());
    }
    logger.log<tfc::logger::lvl_e::debug>("Shutting down signal ({})", signal_number);
    ctx.stop();
  });

  ctx.run();
  return 0;
}
