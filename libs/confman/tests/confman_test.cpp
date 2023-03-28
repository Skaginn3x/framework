#include <boost/ut.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <tfc/progbase.hpp>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/confman/detail/config_rpc_server.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using tfc::confman::observable;
using tfc::confman::config;
using tfc::confman::detail::config_rpc_server;

struct storage {
  observable<int> a{33};
  observable<int> b{44};
  observable<std::string> c{"c"};
};

template<>
struct glz::meta<storage> {
  static constexpr auto value{ glz::object("a", &storage::a, "b", &storage::b, "c", &storage::c) };
};

auto main(int argc, char** argv) -> int {

  tfc::base::init(argc, argv);

  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
    "happy confman"_test = [] {
      asio::io_context ctx{};
      bool a_called{};
      config<storage> config_storage{ctx, "my-key", storage{ .a = {11, [&a_called]([[maybe_unused]] int new_a, int){
                                                                      ut::expect(new_a == 12);
                                                                      a_called = true;
                                                                    }} }};
      config_rpc_server server{ctx};

      asio::deadline_timer update_timer{ ctx };
      update_timer.expires_from_now(boost::posix_time::milliseconds(1));
      update_timer.async_wait([&server, &config_storage](auto) {
        server.update(config_storage.key(), glz::write_json(storage{.a=observable<int>{12}}));
      });

      ctx.run_for(std::chrono::milliseconds(1000)); // todo this is too much time
      ut::expect(a_called);
    };
  };
  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
