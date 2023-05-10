#include <filesystem>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/program_options.hpp>
#include <boost/ut.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/detail/config_dbus_client.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/ipc_connector.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::operator/;
using tfc::confman::config;
using tfc::confman::observable;

struct storage {
  observable<int> a{ 33 };
  observable<int> b{ 44 };
  observable<std::string> c{ "c" };
};

template <>
struct glz::meta<storage> {
  static constexpr auto value{ glz::object("a", &storage::a, "b", &storage::b, "c", &storage::c) };
};

namespace bpo = boost::program_options;

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };
  bool run_slow{};
  desc.add_options()("slow,s", bpo::bool_switch(&run_slow)->default_value(false));
  tfc::base::init(argc, argv, desc);

  boost::asio::io_context ctx{};

  config<storage> conf{ ctx, "bar" };

  ctx.run();

  //
  //  [[maybe_unused]] ut::suite const rpc_test_cases = [] {
  //    ut::skip / "happy path confman"_test = [] {
  //      // remove config file so we can re-run this test
  //      std::error_code ignore{};
  //      std::filesystem::remove(tfc::confman::detail::default_config_filename, ignore);
  //      std::cout << "HERE" << std::endl;
  //      asio::io_context ctx{};
  //      bool a_called{};
  //      std::cout << "HERE" << std::endl;
  //      config_rpc_server server{ ctx };
  //      std::cout << "HERE" << std::endl;
  //      auto defaults = storage{ .a = { 11, [&a_called]([[maybe_unused]] int new_a, int old_a) {
  //                                       ut::expect(old_a == 11);
  //                                       ut::expect(new_a == 12);
  //                                       a_called = true;
  //                                     } } };
  //      std::cout << "HERE" << std::endl;
  //      config<storage> const config_storage(
  //          ctx, "my-key",
  //          [&server](config<storage> const& self) {
  //            // This is the alive callback
  //            ut::expect(self.get().a == 11);  // this is from `defaults` above
  //            ut::expect(self.get().b == storage{}.b);
  //            ut::expect(self.get().c == storage{}.c);
  //            // Here we write new value of `a`
  //            server.update(self.key(), glz::write_json(storage{ .a = observable<int>{ 12 } }));
  //          },
  //          defaults);
  //
  //      std::cout << "HERE" << std::endl;
  //      ctx.run_for(std::chrono::milliseconds(10));
  //      ut::expect(a_called);
  //      std::cout << "HERE" << std::endl;
  //    };
  //    ut::tag("slow") / "get all ipcs"_test = [] {
  //      // remove config file so we can re-run this test
  //      std::error_code ignore{};
  //      std::filesystem::remove(tfc::confman::detail::default_config_filename, ignore);
  //
  //      asio::io_context ctx{};
  //      tfc::ipc::bool_send_exposed const bool_exposed{ ctx, "This_is_a_name" };
  //      tfc::ipc::int_send_exposed const int_exposed{ ctx, "other_name" };
  //      tfc::ipc::bool_recv_conf_cb const bool_configurable{ ctx, "some_other_name", [](auto const&) {} };
  //
  //      config_rpc_server const server{ ctx };
  //
  //      config_rpc_client client{ ctx, "foo" };
  //
  //      boost::asio::steady_timer timer{ ctx };
  //      timer.expires_after(std::chrono::milliseconds(199));
  //      timer.async_wait([&client](auto&&) {
  //        client.request<get_ipcs::tag>(get_ipcs{}, [](std::expected<get_ipcs_result, glz::rpc::error> const& res) {
  //          ut::expect(res.has_value());
  //          if (res.has_value()) {
  //            auto const& vec = res.value();
  //            auto constexpr vec_contains = [](auto&& vector, auto&& contains) {
  //              return std::find_if(vector.begin(), vector.end(),
  //                                  [&contains](auto const& vec_item) { return vec_item.contains(contains); }) !=
  //                                  vector.end();
  //            };
  //            ut::expect(vec_contains(vec, "This_is_a_name"));
  //            ut::expect(vec_contains(vec, "other_name"));
  //            ut::expect(vec_contains(vec, "some_other_name"));
  //          }
  //        });
  //      });
  //
  //      ctx.run_for(std::chrono::milliseconds(200));
  //    };
  //    ut::tag("slow") / "get bool ipcs"_test = [] {
  //      // remove config file so we can re-run this test
  //      std::error_code ignore{};
  //      std::filesystem::remove(tfc::confman::detail::default_config_filename, ignore);
  //
  //      asio::io_context ctx{};
  //      tfc::ipc::bool_send_exposed const bool_exposed{ ctx, "new_name" };
  //
  //      config_rpc_server const server{ ctx };
  //
  //      config_rpc_client client{ ctx, "foo" };
  //
  //      boost::asio::steady_timer timer{ ctx };
  //      timer.expires_after(std::chrono::milliseconds(199));
  //      timer.async_wait([&client](auto&&) {
  //        client.request<get_ipcs::tag>(
  //            get_ipcs{ .type = tfc::ipc::type_e::_bool }, [](std::expected<get_ipcs_result, glz::rpc::error> const& res) {
  //              ut::expect(res.has_value());
  //              if (res.has_value()) {
  //                auto const& vec = res.value();
  //                auto constexpr vec_contains = [](auto&& vector, auto&& contains) {
  //                  return std::find_if(vector.begin(), vector.end(), [&contains](auto const& vec_item) {
  //                           return vec_item.contains(contains);
  //                         }) != vector.end();
  //                };
  //                ut::expect(vec_contains(vec, "new_name"));
  //              }
  //            });
  //      });
  //
  //      ctx.run_for(std::chrono::milliseconds(200));
  //    };
  //  };
  //  if (run_slow) {
  //    ut::cfg<ut::override> = { .tag = { "slow" } };
  //  }
  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
