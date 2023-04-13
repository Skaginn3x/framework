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
    asio::io_context ctx{};

    tfc::confman::detail::config_rpc_server server{ ctx };

    bool called{};
    tfc::ipc::slot_configurable<tfc::ipc::type_json> const foo{ ctx, "foo", [&called](std::string const& value) {
                                                                 expect(value == "helloworld");
                                                                 called = true;
                                                               } };

    std::string signal_name{ "signal" };

    server.update(foo.config().key(),
                  glz::write_json(tfc::ipc::connect_storage{
                      .signal_name = tfc::confman::observable<std::string>{
                          fmt::format("{}.{}.{}", tfc::base::get_exe_name(), tfc::base::get_proc_name(), signal_name) } }));

    auto const my_signal = tfc::ipc::signal<tfc::ipc::type_json>::create(ctx, signal_name).value();

    my_signal->async_send("helloworld", [](auto const&, auto) {});

    ctx.run_for(std::chrono::milliseconds(1000));

    expect(called);
  };

  return 0;
}
