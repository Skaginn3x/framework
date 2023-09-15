#include <chrono>
#include <string>

#include <tfc/ipc.hpp>
#include <tfc/ipc/packet.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/ut.hpp>

namespace asio = boost::asio;

auto main(int, char**) -> int {
  using boost::ut::operator""_test;
  using boost::ut::operator|;
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
    auto sender = tfc::ipc::details::uint_send::create(ctx, "name").value();
    auto receiver = tfc::ipc::details::uint_recv_cb::create(ctx, "name");

    receiver->init(sender->name_w_type(), [](auto val) { std::cout << val << std::endl; });
    asio::deadline_timer verification_timer{ ctx };
    verification_timer.expires_from_now(boost::posix_time::milliseconds(1));
    verification_timer.async_wait([&ctx, &sender](auto) {
      sender->send(10);
      ctx.stop();
    });
    ctx.run();
    expect(true);
  };
  "ipc"_test = [] {
    asio::io_context ctx;
    auto sender = tfc::ipc::details::uint_send::create(ctx, "name").value();
    auto receiver = tfc::ipc::details::uint_recv_cb::create(ctx, "unused");

    constexpr auto time_point_to_uint64 = [](auto const& time_point) -> std::uint64_t {
      auto duration = time_point.time_since_epoch();
      return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
    };

    constexpr auto uint64_to_time_point = [](uint64_t time_stamp) {
      return std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::nanoseconds(time_stamp));
    };

    bool receiver_called{ false };
    receiver->init(sender->name_w_type(), [&ctx, &receiver_called, &uint64_to_time_point](auto val) {
      auto now{ std::chrono::high_resolution_clock::now() };
      auto past{ uint64_to_time_point(val) };
      auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - past).count();
      fmt::print("Round time took: {} ns\n", duration_ns);
      receiver_called = true;
      ctx.stop();
    });
    asio::steady_timer timer{ ctx };
    boost::system::error_code error_code;
    timer.expires_from_now(std::chrono::milliseconds(1), error_code);
    timer.async_wait([&sender, &time_point_to_uint64](auto) {
      auto now{ std::chrono::high_resolution_clock::now() };
      sender->send(time_point_to_uint64(now));
    });

    ctx.run_for(std::chrono::seconds(1));
    expect(receiver_called);
  };

  "code_example"_test = []() {
    auto ctx{ asio::io_context() };
    auto sender{ tfc::ipc::details::string_send::create(ctx, "name").value() };
    auto receiver{ tfc::ipc::details::string_recv_cb::create(ctx, "unused") };
    receiver->init(sender->name_w_type(), [&ctx](std::string const& value) {
      fmt::print("received: {}\n", value);
      ctx.stop();
    });
    asio::steady_timer timer{ ctx };
    timer.expires_from_now(std::chrono::milliseconds(100));
    timer.async_wait([&sender](auto) { sender->send("hello-world"); });
    ctx.run();
  };

  "verify underlying type in type_*"_test = []() {
    expect(std::is_same_v<typename decltype(tfc::ipc::details::type_bool{})::value_t, bool>);
    expect(std::is_same_v<typename decltype(tfc::ipc::details::type_double{})::value_t, double>);
    expect(std::is_same_v<typename decltype(tfc::ipc::details::type_int{})::value_t, int64_t>);
    expect(std::is_same_v<typename decltype(tfc::ipc::details::type_uint{})::value_t, uint64_t>);
    expect(std::is_same_v<typename decltype(tfc::ipc::details::type_json{})::value_t, std::string>);
    expect(std::is_same_v<typename decltype(tfc::ipc::details::type_string{})::value_t, std::string>);
  };
  "type_description from string"_test =
      [](std::pair<tfc::ipc::details::any_type_desc, tfc::ipc::details::type_e> arg) {
        std::visit(
            [&](auto& type_desc) {
              if constexpr (!std::same_as<std::monostate, std::remove_cvref_t<decltype(type_desc)>>) {
                // All parameter strings should produce a type
                expect(type_desc.value_e == arg.second);
              } else {
                expect(false);
              }
            },
            arg.first);
      } |
      std::vector<std::pair<tfc::ipc::details::any_type_desc, tfc::ipc::details::type_e>>{
        { tfc::ipc::details::get_type_description_from_string("string"), tfc::ipc::details::type_e::_string },
        { tfc::ipc::details::get_type_description_from_string("json"), tfc::ipc::details::type_e::_json },
        { tfc::ipc::details::get_type_description_from_string("uint"), tfc::ipc::details::type_e::_uint64_t },
        { tfc::ipc::details::get_type_description_from_string("int"), tfc::ipc::details::type_e::_int64_t },
        { tfc::ipc::details::get_type_description_from_string("double"), tfc::ipc::details::type_e::_double_t },
        { tfc::ipc::details::get_type_description_from_string("bool"), tfc::ipc::details::type_e::_bool }
      };
  return 0;
}
