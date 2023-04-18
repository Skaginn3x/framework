#include <tfc/confman/detail/config_rpc_server.hpp>
#include <tfc/ipc_connector.hpp>
#include <tfc/progbase.hpp>

#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

namespace asio = boost::asio;

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  using boost::ut::operator""_test;
  using boost::ut::expect;
  using tfc::ipc::packet;
  using tfc::ipc::type_e;

  "ipc_connector test"_test = []() {
    // remove config file so we can re-run this test
    std::error_code ignore{};
    std::filesystem::remove(tfc::confman::detail::default_config_filename, ignore);

    asio::io_context ctx{};

    tfc::confman::detail::config_rpc_server server{ ctx };

    bool called{};
    tfc::ipc::slot_configurable<tfc::ipc::type_json> const foo{ ctx, "foo", [&called](std::string const& value) {
                                                                 expect(value == "helloworld");
                                                                 called = true;
                                                               } };

    auto const my_signal = tfc::ipc::signal<tfc::ipc::type_json>::create(ctx, "my_name").value();

    asio::steady_timer timer{ ctx };
    timer.expires_after(std::chrono::milliseconds(50));
    timer.async_wait([&server, &foo, signal_name = my_signal->name_w_type()](auto const&) {
      // Wait for client to be alive otherwise we send notification into the emptiness
      server.update(foo.config().key(),
                    glz::write_json(tfc::ipc::storage::connect{
                        .signal_name = tfc::confman::observable<std::string>{ fmt::format(
                            "{}.{}.{}", tfc::base::get_exe_name(), tfc::base::get_proc_name(), signal_name) } }));
    });

    my_signal->async_send("helloworld", [](auto const&, auto) {});

    ctx.run_for(std::chrono::milliseconds(100));

    expect(called);
  };

  return 0;
}
