#include <memory>
#include <string>
#include <string_view>
#include <variant>

#include <fmt/core.h>
#include <mp-units/format.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <mp-units/ostream.h>

#include <tfc/ipc.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;
namespace ipc = tfc::ipc;

inline auto stdin_coro(asio::io_context &ctx, std::string_view signal_name) -> asio::awaitable<void> {
    auto executor = co_await asio::this_coro::executor;
    asio::posix::stream_descriptor input_stream(executor, STDIN_FILENO);

    auto client{tfc::ipc::make_manager_client(ctx)};

    auto type{ipc::details::enum_cast(signal_name)};
    if (type == ipc::details::type_e::unknown) {
        throw std::runtime_error{fmt::format("Unknown typename in: {}", signal_name)};
    }
    auto sender{ipc::make_any_signal::make(type, ctx, client, signal_name)};
    std::visit(
        []<typename signal_t>(signal_t &my_signal) {
            if constexpr (!std::same_as<std::remove_cvref_t<signal_t>, std::monostate>) {
                fmt::println("Registered signal with name: {}", my_signal.full_name());
            }
        },
        sender);

    while (true) {
        co_await input_stream.async_wait(asio::posix::stream_descriptor::wait_read, asio::use_awaitable);
        std::array<char, 1024> buffer;
        const std::size_t bytes_read = co_await input_stream.async_read_some(asio::buffer(buffer), asio::use_awaitable);
        std::string_view buffer_str{std::begin(buffer), bytes_read - 1}; // strip of the new line character

        std::visit(
            [buffer_str]<typename signal_t>(signal_t &&my_signal) {
                if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<signal_t> >) {
                    using signal_type = std::remove_cvref_t<signal_t>;
                    using value_t = typename signal_type::value_t;
                    std::string buff{buffer_str};
                    value_t val{};
                    if constexpr (signal_type::value_type == ipc::details::type_e::_string ||
                                  signal_type::value_type == ipc::details::type_e::_json) {
                        val = buff;
                    } else {
                        auto value{glz::read_json<value_t>(buff)};
                        if (!value.has_value()) {
                            fmt::println("Invalid input error: {}", glz::format_error(value.error(), buff));
                            return;
                        }
                        val = value.value();
                    }

                    my_signal.async_send(val, [&, actual_value = val](std::error_code const &code, size_t bytes) {
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

// using type_bool = type_description<bool, type_e::_bool>;
// using type_int = type_description<std::int64_t, type_e::_int64_t>;
// using type_uint = type_description<std::uint64_t, type_e::_uint64_t>;
// using type_double = type_description<double, type_e::_double_t>;
// using type_string = type_description<std::string, type_e::_string>;
// using type_json = type_description<std::string, type_e::_json>;
// using mass_t = std::expected<mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, std::int64_t>, mass_error_e>;
// using type_mass = type_description<mass_t, type_e::_mass>;

// auto convert(
//     ipc::details::type_e in_type) -> std::variant<ipc::details::type_bool, ipc::details::type_int,
//     ipc::details::type_uint, ipc::details::type_double, ipc::details::type_string, ipc::details::type_json,
//     ipc::details::type_mass> {
//     switch (in_type) {
//         case ipc::details::type_e::_bool: {
//             return ipc::details::type_bool;
//         }
//         case ipc::details::type_e::_int64_t: {
//             return ipc::details::type_int;
//         }
//         case ipc::details::type_e::_uint64_t: {
//             return ipc::details::type_uint;
//         }
//         case ipc::details::type_e::_double_t: {
//             return ipc::details::type_double{};
//         }
//         case ipc::details::type_e::_string: {
//             return ipc::details::type_string{};
//         }
//         case ipc::details::type_e::_json: {
//             return ipc::details::type_json{};
//         }
//         case ipc::details::type_e::_mass: {
//             return ipc::details::type_mass{};
//         }
//     }
// }

auto main(int argc, char **argv) -> int {
    auto description{tfc::base::default_description()};

    std::string signal{};
    std::string slot{};
    std::vector<std::string> connect;
    bool list_signals{};
    bool list_slots{};

    description.add_options()("signal", po::value<std::string>(&signal), "IPC signal channel (output)")(
        "slot", po::value<std::string>(&slot), "IPC slot channel (input)")(
        "connect,c", po::value<std::vector<std::string> >(&connect)->multitoken(), "Listen to these slots")(
        "list-signals", po::bool_switch(&list_signals), "List all available IPC signals")(
        "list-slots", po::bool_switch(&list_slots), "List all available IPC slots");
    tfc::base::init(argc, argv, description);

    bool at_least_one_choice{!signal.empty() || !slot.empty() || !connect.empty() || list_signals || list_slots};
    if (!at_least_one_choice) {
        std::stringstream out;
        description.print(out);
        fmt::println("Usage: tfcctl [options] \n{}", out.str());
        std::exit(0);
    }

    asio::io_context ctx;

    if (list_signals || list_slots) {
        auto client{std::make_shared<tfc::ipc_ruler::ipc_manager_client>(ctx)};
        const auto print_names{
            [client](std::string context, auto const &instances) {
                [[maybe_unused]] auto foo{client}; // prevent warnings
                fmt::println("{}", context);
                for (const auto &instance: instances) {
                    fmt::println("{}", instance.name);
                }
            }
        };
        if (list_signals) {
            client->signals(std::bind_front(print_names, "Signals:"));
        }
        if (list_slots) {
            client->slots(std::bind_front(print_names, "Slots:"));
        }
    }

    // For sending a signal
    if (!signal.empty()) {
        asio::co_spawn(ctx, stdin_coro(ctx, signal), asio::detached);
    }

    std::vector<tfc::ipc::details::any_slot_cb> connect_slots;

    auto constexpr slot_connect{
        [](auto &&receiver_variant, std::string_view signal_name) {
            std::visit(
                [signal_name]<typename receiver_t>(receiver_t &&receiver) {
                    if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<receiver_t> >) {
                        fmt::println("Connecting to signal {}", signal_name);
                        std::string sig{signal_name};
                        auto error = receiver->connect(signal_name, [sig]<typename val_t>(val_t const &val) {
                            if constexpr (tfc::stx::is_expected<std::remove_cvref_t<val_t> >) {
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
        }
    };

    for (auto &signal_connect: connect) {
        // For listening to connections
        connect_slots.emplace_back([&ctx, slot_connect](std::string_view sig) -> tfc::ipc::details::any_slot_cb {
            std::string const slot_name = fmt::format("tfcctl_slot_{}", sig);
            auto const type{ipc::details::enum_cast(sig)};
            if (type == ipc::details::type_e::unknown) {
                throw std::runtime_error{fmt::format("Unknown typename in: {}", sig)};
            }
            auto ipc{ipc::details::make_any_slot_cb::make(type, ctx, slot_name)};
            slot_connect(ipc, sig);
            return ipc;
        }(signal_connect));
    }

    if (!slot.empty()) {
        std::string const slot_name = fmt::format("tfcctl_slot_{}", slot);
        auto const type{ipc::details::enum_cast(slot)};
        if (type == ipc::details::type_e::unknown) {
            throw std::runtime_error{fmt::format("Unknown typename in: {}", slot)};
        }
        tfc::ipc_ruler::ipc_manager_client client(ctx);

        if (type == tfc::ipc::details::type_e::_bool) {
            ipc::slot<ipc::details::type_bool> my_bool_slot{
                ctx, client, slot, "description", [](bool new_value) {
                    fmt::println("New value: {}", new_value);
                }
            };
            ctx.run();
        }
    }


    ctx.run();

    return 0;
}
