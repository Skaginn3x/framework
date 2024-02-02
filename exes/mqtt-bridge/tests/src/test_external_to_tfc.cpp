#include <chrono>
#include <string>
#include <variant>

#include <boost/asio.hpp>
#include <boost/ut.hpp>

#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>

#include <config/bridge_mock.hpp>
#include <external_to_tfc.hpp>
#include <test_external_to_tfc.hpp>

namespace tfc::mqtt {
    namespace asio = boost::asio;
    namespace ut = boost::ut;

    auto test_external_to_tfc::test() -> bool {
        asio::io_context isolated_ctx{};

        using ipc::signal;
        using ipc::slot;
        using ipc_ruler::ipc_manager_client_mock;
        using namespace tfc::ipc::details;
        using std::chrono::milliseconds;

        ipc_manager_client_mock ipc_client{isolated_ctx};
        config::bridge_mock config{isolated_ctx, "test"};
        config.add_writeable_signal("test_signal", "test_signal", type_e::_bool);
        isolated_ctx.run_for(milliseconds{1});

        external_to_tfc<ipc_manager_client_mock &, config::bridge_mock> ext_test{isolated_ctx, config, ipc_client};
        isolated_ctx.run_for(milliseconds{1});

        const slot<type_bool, ipc_manager_client_mock &> recv_slot(isolated_ctx, ext_test.ipc_client_, "test_slot", "",
                                                                   [&](bool) {
                                                                   });

        isolated_ctx.run_for(milliseconds{1});

        ext_test.create_outward_signals();

        isolated_ctx.run_for(milliseconds{1});

        ext_test.ipc_client_.connect(ext_test.ipc_client_.slots_[0].name, ext_test.ipc_client_.signals_[0].name,
                                     [&](std::error_code) {
                                     });

        isolated_ctx.run_for(milliseconds{1});

        ext_test.receive_new_value("test_signal", true);

        isolated_ctx.run_for(milliseconds{1});

        if (ext_test.ipc_client_.signals_[0].name != "test_mqtt_bridge.def.bool.test_signal") {
            return false;
        }

        if (ext_test.ipc_client_.slots_[0].name != "test_mqtt_bridge.def.bool.test_slot") {
            return false;
        }

        if (!recv_slot.value().has_value()) {
            return false;
        }

        return recv_slot.value().value();
    }
} // namespace tfc::mqtt
