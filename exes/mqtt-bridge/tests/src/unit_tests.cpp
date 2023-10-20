#include <chrono>
#include <string>

#include <async_mqtt/all.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/ut.hpp>

#include <client.hpp>
#include <constants.hpp>
#include <spark_plug_interface.hpp>
#include <test_external_to_tfc.hpp>
#include <test_tfc_to_external.hpp>
#include <tfc/progbase.hpp>

namespace ut = boost::ut;
namespace asio = boost::asio;

using ut::operator""_test;
using ut::operator>>;
using ut::expect;
using ut::fatal;

auto main(int argc, char* argv[]) -> int {
  auto program_description{ tfc::base::default_description() };
  tfc::base::init(argc, argv, program_description);
  asio::io_context io_ctx;

  "testing tfc to external"_test = [&]() {
    tfc::mqtt::test_tfc_to_external test_ext{};

    bool set_signals;
    asio::co_spawn(
        io_ctx, [&]() -> asio::awaitable<void> { set_signals = co_await test_ext.test(); }, asio::detached);
    io_ctx.run_for(std::chrono::milliseconds(10));
    expect(set_signals);
  };

  "testing client"_test = [&]() {
    const std::string will_topic = "will";
    const std::string will_payload = "payload";

    tfc::mqtt::client_mock cli{ io_ctx, will_topic, will_payload };

    bool connect;
    asio::co_spawn(
        io_ctx, [&]() -> asio::awaitable<void> { connect = co_await cli.connect(); }, asio::detached);
    io_ctx.run_for(std::chrono::milliseconds(10));
    expect(connect);

    bool send_message;
    asio::co_spawn(
        io_ctx,
        [&]() -> asio::awaitable<void> {
          send_message = co_await cli.send_message("topic", "payload", async_mqtt::qos::at_least_once);
        },
        asio::detached);
    io_ctx.run_for(std::chrono::milliseconds(10));
    expect(send_message);
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
    tfc::mqtt::spark_plug_mock sp{ io_ctx };

    expect(sp.type_enum_convert(tfc::ipc::details::type_e::_bool) == 11);
    expect(sp.type_enum_convert(tfc::ipc::details::type_e::_double_t) == 10);

    expect(sp.format_signal_name("tfc.bool.test.something") == "tfc/bool/test/something");
    expect(sp.topic_formatter({ "first", "second" }) == "first/second");
  };

  "testing external to tfc"_test = [&]() {
    tfc::mqtt::test_external_to_tfc test_ext{};
    ut::expect(test_ext.test());
  };
}
