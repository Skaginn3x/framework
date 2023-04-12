#include <filesystem>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/ut.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/detail/config_rpc_server.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using tfc::confman::config;
using tfc::confman::observable;
using tfc::confman::detail::config_rpc_server;

struct storage {
  observable<int> a{ 33 };
  observable<int> b{ 44 };
  observable<std::string> c{ "c" };
};

template <>
struct glz::meta<storage> {
  static constexpr auto value{ glz::object("a", &storage::a, "b", &storage::b, "c", &storage::c) };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
    "happy path confman"_test = [] {
      // remove config file so we can re-run this test
      std::error_code ignore{};
      std::filesystem::remove(tfc::confman::detail::default_config_filename, ignore);

      asio::io_context ctx{};
      bool a_called{};
      config_rpc_server server{ ctx };
      auto defaults = storage{ .a = { 11, [&a_called]([[maybe_unused]] int new_a, int old_a) {
                                       ut::expect(old_a == 11);
                                       ut::expect(new_a == 12);
                                       a_called = true;
                                     } } };
      config<storage> const config_storage(
          ctx, "my-key",
          [&server](config<storage> const& self) {
            // This is the alive callback
            ut::expect(self.get().a == 11);  // this is from `defaults` above
            ut::expect(self.get().b == storage{}.b);
            ut::expect(self.get().c == storage{}.c);
            // Here we write new value of `a`
            server.update(self.key(), glz::write_json(storage{ .a = observable<int>{ 12 } }));
          },
          defaults);

      ctx.run_for(std::chrono::milliseconds(10));
      ut::expect(a_called);
    };
  };
  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
