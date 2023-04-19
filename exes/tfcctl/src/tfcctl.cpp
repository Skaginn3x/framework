#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>

#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <tfc/confman/detail/config_rpc_client.hpp>
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

  auto sender{ tfc::ipc::create_ipc_send<tfc::ipc::any_send>(ctx, signal_name) };

  while (true) {
    co_await input_stream.async_wait(asio::posix::stream_descriptor::wait_read, asio::use_awaitable);
    std::array<char, 1024> buffer;
    const std::size_t bytes_read = co_await input_stream.async_read_some(asio::buffer(buffer), asio::use_awaitable);
    std::string_view buffer_str{ std::begin(buffer), bytes_read - 1 };  // strip of the new line character

    constexpr auto send{ [](std::string_view input, auto in_sender, auto& in_logger) -> void {
      try {
        using value_t = typename decltype(in_sender)::element_type::value_t;
        auto value = boost::lexical_cast<value_t>(input);
        in_sender->async_send(value, [&, value](std::error_code code, size_t bytes) {
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
    if (auto* bool_sender{ std::get_if<std::shared_ptr<ipc::bool_send>>(&sender) }) {
      send(buffer_str, *bool_sender, logger);
    } else if (auto* int_sender{ std::get_if<std::shared_ptr<ipc::int_send>>(&sender) }) {
      send(buffer_str, *int_sender, logger);
    } else if (auto* uint_sender{ std::get_if<std::shared_ptr<ipc::uint_send>>(&sender) }) {
      send(buffer_str, *uint_sender, logger);
    } else if (auto* double_sender{ std::get_if<std::shared_ptr<ipc::double_send>>(&sender) }) {
      send(buffer_str, *double_sender, logger);
    } else if (auto* string_sender{ std::get_if<std::shared_ptr<ipc::string_send>>(&sender) }) {
      send(buffer_str, *string_sender, logger);
    } else if (auto* json_sender{ std::get_if<std::shared_ptr<ipc::json_send>>(&sender) }) {
      send(buffer_str, *json_sender, logger);
    }
  }
}

auto main(int argc, char** argv) -> int {
  auto description{ tfc::base::default_description() };

  std::string signal{};
  std::string slot{};

  std::vector<std::string> connect;

  po::options_description ipc_desc("ipc");
  ipc_desc.add_options()("signal", po::value<std::string>(&signal), "IPC signal channel (output)")(
      "slot", po::value<std::string>(&slot), "IPC slot channel (input)")(
      "connect,c", po::value<std::vector<std::string>>(&connect)->multitoken(), "Listen to these slots");

  bool get_slots{};
  bool get_signals{};

  po::options_description config_desc("config");
  config_desc.add_options()("get_slots", po::bool_switch(&get_slots)->default_value(false), "Get IPC slots")(
      "get_signals", po::bool_switch(&get_signals)->default_value(false), "Get IPC signals");

  description.add(ipc_desc).add(config_desc);
  tfc::base::init(argc, argv, description);

  // Must provide an argument
  //    if (tfc::base::get_map().find("signal") == tfc::base::get_map().end() && connect.empty()) {
  //      std::stringstream out;
  //      description.print(out);
  //      fmt::print("Usage: tfcctl [options] \n{}", out.str());
  //      std::exit(0);
  //    }
  tfc::logger::logger logger{ "tfc control" };

  asio::io_context ctx;

  // For sending a signal
  if (!signal.empty()) {
    asio::co_spawn(ctx, stdin_coro(ctx, logger, signal), asio::detached);
  }

  std::vector<tfc::ipc::any_recv_cb> connect_slots;
  for (auto& signal_connect : connect) {
    // For listening to connections
    connect_slots.emplace_back([&ctx, &logger](std::string_view sig) -> tfc::ipc::any_recv_cb {
      std::string slot_name = fmt::format("tfcctl_slot_{}", sig);
      auto ipc{ tfc::ipc::create_ipc_recv_cb<tfc::ipc::any_recv_cb>(ctx, slot_name) };
      std::visit(
          [&](auto&& receiver) {
            using receiver_t = std::remove_cvref_t<decltype(receiver)>;
            if constexpr (!std::same_as<std::monostate, receiver_t>) {
              logger.log<tfc::logger::lvl_e::trace>("Connecting to signal {}", slot_name, sig);
              auto error = receiver->init(
                  sig, [&, sig](auto const& val) { logger.log<tfc::logger::lvl_e::info>("{}: {}", sig, val); });
              if (error) {
                logger.log<tfc::logger::lvl_e::error>("Failed to connect: {}", error.message());
              }
            }
          },
          ipc);
      return ipc;
    }(signal_connect));
  }

  std::unique_ptr<tfc::confman::detail::config_rpc_client> client{};
  auto constexpr print_ipcs{ [](std::string const& direction) {
    return [direction](std::expected<tfc::confman::detail::method::get_ipcs_result, glz::rpc::error> const& res) {
      if (res) {
        fmt::print("{}:\n{}\n", direction, fmt::join(res.value(), "\n"));
      } else {
        fmt::print("Error fetching {}: \"{}\"\n\"{}\"\n", direction, res.error().get_message(), res.error().get_data());
      }
    };
  } };
  if (get_slots) {
    client = std::make_unique<tfc::confman::detail::config_rpc_client>(ctx, "tfcctl");
    client->request<tfc::confman::detail::method::get_ipcs::tag>(
        tfc::confman::detail::method::get_ipcs{ .direction = tfc::ipc::direction_e::slot }, print_ipcs("Slots"));
  }
  if (get_signals) {
    if (client == nullptr) {
      client = std::make_unique<tfc::confman::detail::config_rpc_client>(ctx, "tfcctl");
    }
    client->request<tfc::confman::detail::method::get_ipcs::tag>(
        tfc::confman::detail::method::get_ipcs{ .direction = tfc::ipc::direction_e::signal }, print_ipcs("Signals"));
  }

  //  asio::signal_set signal_set{ ctx, SIGINT, SIGTERM, SIGHUP };
  //  signal_set.async_wait([&](const std::error_code& error, int signal_number) {
  //    if (error) {
  //      logger.log<tfc::logger::lvl_e::error>("Error waiting for signal. {}", error.message());
  //    }
  //    logger.log<tfc::logger::lvl_e::debug>("Shutting down signal ({})", signal_number);
  //    ctx.stop();
  //  });

  ctx.run();
  return 0;
}
