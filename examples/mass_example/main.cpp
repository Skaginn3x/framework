#include <chrono>
#include <iostream>
#include <system_error>

#include <tfc/ipc.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/progbase.hpp>

#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>

namespace asio = boost::asio;

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  auto ctx{ asio::io_context() };
  tfc::ipc_ruler::ipc_manager_client_mock ipc_client_{ ctx };

 //   tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> weight_signal{
 //     ctx,
 //     ipc_client_,
 //     "weight",
 //   };

 //   tfc::ipc::slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> weight_slot{ ctx, ipc_client_,
 //                                                                                                       "weight",
 //                                                                                                       [](bool) {} };

 //   ipc_client_.connect(ipc_client_.slots_[0].name, ipc_client_.signals_[0].name, [](std::error_code const&) {});

 //   weight_signal.send(true);

 //   ctx.run_for(std::chrono::milliseconds(10));

 //   std::cout << weight_slot.value().value() << std::endl;

      tfc::ipc::signal<tfc::ipc::details::type_mass, tfc::ipc_ruler::ipc_manager_client_mock&> weight_signal{
        ctx,
        ipc_client_,
        "weight",
      };

      tfc::ipc::slot<tfc::ipc::details::type_mass, tfc::ipc_ruler::ipc_manager_client_mock&> weight_slot{
        ctx, ipc_client_, "weight", [](const tfc::ipc::details::mass_t&) {}
      };

      ipc_client_.connect(ipc_client_.slots_[0].name, ipc_client_.signals_[0].name, [](std::error_code const&) {});

      weight_signal.send(100 * mp_units::si::gram);

      ctx.run_for(std::chrono::milliseconds(10));

      auto check_value = [&weight_slot]() -> bool {
        // check if std::optional contains value
        if (weight_slot.value().has_value()) {
          // check if std::expected contains value
          if (weight_slot.value()->has_value()) {
            return weight_slot.value()->value() == 100 * mp_units::si::gram;
          }
        }
        return false;
      };

   std::cout << "Value arrived on slot: " << (check_value() ? "true" : "false") << std::endl;

  return EXIT_SUCCESS;
}
