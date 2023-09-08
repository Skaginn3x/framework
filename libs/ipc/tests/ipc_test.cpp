#include <chrono>
#include <string>

#include <tfc/ipc.hpp>
#include <tfc/ipc/packet.hpp>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <boost/asio/deadline_timer.hpp>
#include <boost/ut.hpp>

namespace asio = boost::asio;

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  using boost::ut::bdd::given;
  using boost::ut::bdd::when;
  using boost::ut::operator>>;
  using boost::ut::fatal;
  using tfc::ipc::details::packet;
  using tfc::ipc::details::type_e;

  "packet_serialization"_test = []() {
    constexpr auto deserialize_serialize{ [](auto&& pack) {
      using pack_t = std::remove_cvref_t<decltype(pack)>;
      using packet_t = packet<typename pack_t::value_t, pack_t::type_v>;
      std::vector<std::byte> serialized{};
      auto err{ packet_t::serialize(pack.value, serialized) };
      expect(!err >> fatal);
      auto supposed_value = packet_t::deserialize(std::span(std::cbegin(serialized), std::cend(serialized)));

      expect(supposed_value.has_value() >> fatal);
      expect(pack.value == supposed_value.value());
    } };
    given("serialization") = [&deserialize_serialize] {
      when("bool=true") = [&deserialize_serialize] { deserialize_serialize(packet<bool, type_e::_bool>{ .value = true }); };
      when("bool=false") = [&deserialize_serialize] {
        deserialize_serialize(packet<bool, type_e::_bool>{ .value = false });
      };
      when("int=-1337") = [&deserialize_serialize] {
        deserialize_serialize(packet<std::int64_t, type_e::_int64_t>{ .value = -1337 });
      };
      when("int=1337") = [&deserialize_serialize] {
        deserialize_serialize(packet<std::int64_t, type_e::_int64_t>{ .value = 1337 });
      };
      when("uint=max") = [&deserialize_serialize] {
        deserialize_serialize(
            packet<std::uint64_t, type_e::_uint64_t>{ .value = std::numeric_limits<std::uint64_t>::max() });
      };
      when("double=4.21337") = [] {
        packet<std::double_t, type_e::_double_t> pack{ .value = 4.21337 };

        using pack_t = std::remove_cvref_t<decltype(pack)>;
        using packet_t = packet<typename pack_t::value_t, pack_t::type_v>;
        std::vector<std::byte> serialized{};
        auto err{ packet_t::serialize(pack.value, serialized) };
        expect(!err >> fatal);
        auto supposed_value = packet_t::deserialize(std::span(std::cbegin(serialized), std::cend(serialized)));

        expect(supposed_value.has_value() >> fatal);
        // unsafe equal comparison of float
        //        expect(pack.value == supposed_packet.value);
        //        expect(pack == supposed_packet);
        expect(supposed_value.value() > 4.2 && supposed_value.value() < 4.22);
      };
      when("string=hello world from another world") = [&deserialize_serialize] {
        deserialize_serialize(packet<std::string, type_e::_string>{ .value = "hello world from another world" });
      };
      when(R"(json={"i":287,"d":3.14,"hello":"Hello World","arr":[1,2,3])") = [&deserialize_serialize] {
        deserialize_serialize(
            packet<std::string, type_e::_json>{ .value = R"({"i":287,"d":3.14,"hello":"Hello World","arr":[1,2,3])" });
      };
    };
  };

  "ipc stop receiver"_test = [] {
    asio::io_context ctx;
    auto sender = tfc::ipc::details::uint_signal_ptr::element_type::create(ctx, "name").value();
    auto receiver = tfc::ipc::details::uint_slot_cb_ptr::element_type::create(
        ctx, "name", [](auto val) { std::cout << val << std::endl; });

    receiver->connect(sender->name_w_type());
    asio::steady_timer verification_timer{ ctx };
    verification_timer.expires_after(std::chrono::milliseconds{ 1 });
    verification_timer.async_wait([&ctx, &sender](auto) {
      sender->send(10);
      ctx.stop();
    });
    ctx.run();
    expect(true);
  };
  "ipc"_test = [] {
    asio::io_context ctx;
    constexpr auto time_point_to_uint64 = [](auto const& time_point) -> std::uint64_t {
      auto duration = time_point.time_since_epoch();
      return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    };
    constexpr auto uint64_to_time_point = [](uint64_t time_stamp) {
      return std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::nanoseconds(time_stamp));
    };

    auto sender = tfc::ipc::details::uint_signal_ptr::element_type::create(ctx, "name").value();
    bool receiver_called{ false };
    auto receiver = tfc::ipc::details::uint_slot_cb_ptr::element_type::create(ctx, "unused", [&ctx, &receiver_called, &uint64_to_time_point](auto val) {
      auto now{ std::chrono::high_resolution_clock::now() };
      auto past{ uint64_to_time_point(val) };
      auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - past);
      fmt::print("Round time took: {}\n", duration_ns);
      receiver_called = true;
      ctx.stop();
    });
    receiver->connect(sender->name_w_type());
    asio::steady_timer timer{ ctx };
    timer.expires_after(std::chrono::milliseconds(1));
    timer.async_wait([&sender, &time_point_to_uint64](auto) {
      auto now{ std::chrono::high_resolution_clock::now() };
      sender->send(time_point_to_uint64(now));
    });

    ctx.run_for(std::chrono::seconds(1));
    expect(receiver_called);
  };

  "code_example"_test = []() {
    auto ctx{ asio::io_context() };
    auto sender{ tfc::ipc::details::string_signal_ptr::element_type::create(ctx, "name").value() };
    auto receiver{ tfc::ipc::details::string_slot_cb_ptr::element_type::create(ctx, "unused", [&ctx](std::string const& value) {
      fmt::print("received: {}\n", value);
      ctx.stop();
    }) };
    receiver->connect(sender->name_w_type());
    asio::steady_timer timer{ ctx };
    timer.expires_after(std::chrono::milliseconds(100));
    timer.async_wait([&sender](auto) { sender->send("hello-world"); });
    ctx.run();
  };

  return 0;
}
