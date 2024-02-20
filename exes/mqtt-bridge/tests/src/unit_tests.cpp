#include <chrono>
#include <string>

#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/ut.hpp>

#include <tfc/progbase.hpp>

#include <client.hpp>
#include <constants.hpp>
#include <endpoint_mock.hpp>
#include <spark_plug_interface.hpp>
#include <test_external_to_tfc.hpp>
#include <test_tfc_to_external.hpp>

#include <config/bridge_mock.hpp>

namespace ut = boost::ut;
namespace asio = boost::asio;

using ut::operator""_test;
using ut::expect;

auto main(int argc, char *argv[]) -> int {
    auto program_description{tfc::base::default_description()};
    tfc::base::init(argc, argv, program_description);
    asio::io_context io_ctx;

    "testing tfc to external"_test = [&]() {
        tfc::mqtt::test_tfc_to_external test_ext{};

        bool set_signals;
        co_spawn(
            io_ctx,
            [&]() -> asio::awaitable<void> {
                set_signals = co_await test_ext.test();
                io_ctx.stop();
            },
            asio::detached);
        io_ctx.run_for(std::chrono::seconds{10});
        expect(set_signals);
    };

    "testing constants"_test = [&]() {
        expect(tfc::mqtt::constants::namespace_element == "spBv1.0");
        expect(tfc::mqtt::constants::ndata == "NDATA");
        expect(tfc::mqtt::constants::nbirth == "NBIRTH");
        expect(tfc::mqtt::constants::ndeath == "NDEATH");
        expect(tfc::mqtt::constants::ncmd == "NCMD");
        expect(tfc::mqtt::constants::rebirth_metric == "Node Control/Rebirth");
    };

    "testing spark plug interface"_test = [&]() {
        tfc::mqtt::config::bridge_mock config{io_ctx, "test"};
        tfc::mqtt::spark_plug_interface<tfc::mqtt::config::bridge_mock, tfc::mqtt::client<
            tfc::mqtt::endpoint_client_mock, tfc::mqtt::config::bridge_mock> > sp{io_ctx, config};

        expect(sp.topic_formatter({"first", "second"}) == "first/second");
    };

    "testing tfc to external"_test = [&]() {
        tfc::ipc_ruler::ipc_manager_client_mock ipc_mock{io_ctx};

        tfc::mqtt::config::bridge_mock config{io_ctx, "test"};
        tfc::mqtt::spark_plug_interface<tfc::mqtt::config::bridge_mock, tfc::mqtt::client<
            tfc::mqtt::endpoint_client_mock, tfc::mqtt::config::bridge_mock> > sp{io_ctx, config};

        tfc::mqtt::tfc_to_external<tfc::mqtt::config::bridge_mock, tfc::mqtt::client<tfc::mqtt::endpoint_client_mock,
            tfc::mqtt::config::bridge_mock>, tfc::ipc_ruler::ipc_manager_client_mock &> test_ext{
            io_ctx, sp, ipc_mock, config
        };

        expect(test_ext.type_enum_convert(tfc::ipc::details::type_e::_bool) == 11);
        expect(test_ext.type_enum_convert(tfc::ipc::details::type_e::_double_t) == 10);
        expect(test_ext.format_signal_name("tfc.bool.test.something") == "tfc/bool/test/something");
    };

    "testing external to tfc"_test = [&]() {
        tfc::mqtt::test_external_to_tfc test_ext{};
        expect(test_ext.test());
    };

    "testing external to tfc - last word"_test = [&]() {
        tfc::mqtt::test_external_to_tfc test_ext{};
        expect(test_ext.test_last_word("tfc/bool/test/something", "something"));
        expect(test_ext.test_last_word("tfc/something", "something"));
        expect(test_ext.test_last_word("trailing/delimiter/", ""));
        expect(test_ext.test_last_word("", std::nullopt));
        expect(test_ext.test_last_word("a", std::nullopt));
        expect(test_ext.test_last_word("/", ""));
        expect(test_ext.test_last_word("multiple//delimiters", "delimiters"));
    };
}
