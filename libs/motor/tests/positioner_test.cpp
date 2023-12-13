#include <iostream>

#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <tfc/motors/positioner.hpp>
#include <tfc/progbase.hpp>

#include <tfc/ipc/details/dbus_client_iface_mock.hpp>

#include <mp-units/systems/si/si.h>

using mp_units::quantity;
using mp_units::si::unit_symbols::mm;
using tfc::motor::detail::tacho_config;

namespace asio = boost::asio;

class single_tacho_config_mock {
public:
  single_tacho_config_mock(asio::io_context&, std::string_view) {}

  tacho_config tacho{ tacho_config::one_tacho };
  quantity<mm> displacement_per_pulse{ 1 * mm };

  // Overload the dereference operator
  single_tacho_config_mock* operator->() { return this; }
};

class double_tacho_config_mock {
public:
  double_tacho_config_mock(asio::io_context&, std::string_view) {}

  tacho_config tacho{ tacho_config::two_tacho };
  quantity<mm> displacement_per_pulse{ 1 * mm };

  // Overload the dereference operator
  double_tacho_config_mock* operator->() { return this; }
};

int main(int argc, char** argv) {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  tfc::base::init(argc, argv);

  /// These are the tests for the tachometer
  /// Only one value can be updated at a time
  ///	new	new	old	old
  ///	pin1    pin2    pin1    pin2    Result
  ///	----	----	----	----	------
  ///	0       0	0       0	no movement
  ///	1       0	0       0	-1
  ///	0       1	0       0	+1
  ///	0       0	1       0	+1
  ///	0       0	0       1	-1
  ///	1       0	1       0	no movement
  ///	1       0	1       1	+1
  ///	0       1	0       1	no movement
  ///	0       1	1       1	-1
  ///	1       1	1       0	-1
  ///	1       1	0       1	+1
  ///	1       1	1       1	no movement

  ///	0   0   |   0   0   ->  no movement
  "tachometer: 0"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    expect(tacho.position_ == 0);

    // No movement
    tacho.first_tacho_update(false);
    tacho.second_tacho_update(false);

    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   0   ->  -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.first_tacho_update(true);

    expect(tacho.position_ == -1);
  };

  ///	1   0   |   0   0   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.second_tacho_update(true);

    expect(tacho.position_ == 1);
  };

  ///	0   0   |   0   1   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.first_tacho_update(true);

    expect(tacho.position_ == -1);

    tacho.first_tacho_update(false);

    expect(tacho.position_ == 0);
  };

  ///	0   0   |   1   0   ->   -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.second_tacho_update(true);

    expect(tacho.position_ == 1);

    tacho.second_tacho_update(false);

    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   1   ->   no movement
  "tachometer: 0"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.first_tacho_update(true);

    expect(tacho.position_ == -1);

    tacho.first_tacho_update(true);

    expect(tacho.position_ == -1);
  };

  ///	0   1   |   1   1   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);

    expect(tacho.position_ == -2) << tacho.position_;

    tacho.second_tacho_update(false);
    expect(tacho.position_ == -1);
  };

  ///	1   0   |   1   0   ->   no movement
  "tachometer: 0"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.second_tacho_update(true);

    expect(tacho.position_ == 1);

    tacho.second_tacho_update(true);

    expect(tacho.position_ == 1);
  };

  ///	1   0   |   1   1   ->   -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);

    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(false);

    expect(tacho.position_ == -3);
  };

  ///   1   1   |   0   1   ->   -1
  "tachometer: -1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.first_tacho_update(true);

    expect(tacho.position_ == -1) << tacho.position_;

    tacho.second_tacho_update(true);

    expect(tacho.position_ == -2) << tacho.position_;
  };

  ///   1   1   |   1   0   ->   +1
  "tachometer: +1"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.second_tacho_update(true);

    expect(tacho.position_ == 1) << tacho.position_;

    tacho.first_tacho_update(true);

    expect(tacho.position_ == 2) << tacho.position_;
  };

  ///   1   1   |   1   1   ->   no movement
  "tachometer: 0"_test = [] {
    tfc::motor::detail::double_tachometer tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);

    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;
  };

  "test callback function"_test = [] {
    std::int64_t callback_counter = 0;

    std::function<void(std::int64_t)> callback{ [&callback_counter](std::int64_t) { callback_counter++; } };

    tfc::motor::detail::double_tachometer tacho{ callback };

    tacho.first_tacho_update(true);

    expect(callback_counter == 1);

    tacho.second_tacho_update(true);

    expect(callback_counter == 2);
  };

  "test positioner"_test = [] {
    using mp_units::si::unit_symbols::mm;

    asio::io_context io_context{};
    tfc::ipc_ruler::ipc_manager_client_mock ipc_manager_client{ io_context };
    tfc::motor::positioner<tfc::ipc_ruler::ipc_manager_client_mock&, single_tacho_config_mock> positioner{
      io_context, ipc_manager_client, "positioner_name"
    };

    expect(positioner.position() == 0 * mm);

    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> signal{ io_context,
                                                                                                     ipc_manager_client,
                                                                                                     "tacho_signal" };

    ipc_manager_client.connect(ipc_manager_client.slots_[0].name, signal.full_name(), [](std::error_code) {});

    io_context.run_for(std::chrono::milliseconds(100));

    signal.send(true);

    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == 1 * mm) << positioner.position().numerical_value_;

    signal.send(false);

    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == 1 * mm) << positioner.position().numerical_value_;

    signal.send(true);

    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == 2 * mm) << positioner.position().numerical_value_;
  };

  "test notifications and single tacho"_test = [] {
    using mp_units::si::unit_symbols::mm;
    using mp_units::quantity;

    asio::io_context io_context{};
    tfc::ipc_ruler::ipc_manager_client_mock ipc_manager_client{ io_context };

    tfc::motor::positioner<tfc::ipc_ruler::ipc_manager_client_mock&, single_tacho_config_mock> positioner{
      io_context, ipc_manager_client, "positioner_name"
    };

    std::int64_t callback_counter = 0;

    std::function<void(quantity<mm>)> callback{ [&callback_counter](quantity<mm>) { callback_counter++; } };

    quantity<mm> position_1mm{ 1 * mm };
    quantity<mm> position_2mm{ 2 * mm };
    quantity<mm> position_3mm{ 3 * mm };

    positioner.notify_after(position_3mm, callback);
    positioner.notify_after(position_2mm, callback);
    positioner.notify_after(position_1mm, callback);

    expect(positioner.position() == 0 * mm);

    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> signal{ io_context,
                                                                                                     ipc_manager_client,
                                                                                                     "tacho_signal" };

    ipc_manager_client.connect(ipc_manager_client.slots_[0].name, signal.full_name(), [](std::error_code) {});

    io_context.run_for(std::chrono::milliseconds(100));

    signal.send(true);
    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == 1 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 1);

    signal.send(false);
    io_context.run_for(std::chrono::milliseconds(100));

    signal.send(true);
    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == 2 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 2);

    signal.send(false);
    io_context.run_for(std::chrono::milliseconds(100));

    signal.send(true);
    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == 3 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 3);
  };

  "test double tacho"_test = [] {
    using mp_units::si::unit_symbols::mm;
    using mp_units::quantity;

    asio::io_context io_context{};
    tfc::ipc_ruler::ipc_manager_client_mock ipc_manager_client{ io_context };

    tfc::motor::positioner<tfc::ipc_ruler::ipc_manager_client_mock&, double_tacho_config_mock> positioner{
      io_context, ipc_manager_client, "positioner_name"
    };

    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == 0 * mm);

    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> signal_a{ io_context,
                                                                                                       ipc_manager_client,
                                                                                                       "tacho_signal_a" };
    io_context.run_for(std::chrono::milliseconds(100));

    tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&> signal_b{ io_context,
                                                                                                       ipc_manager_client,
                                                                                                       "tacho_signal_b" };
    io_context.run_for(std::chrono::milliseconds(100));

    ipc_manager_client.connect(ipc_manager_client.slots_[0].name, signal_a.full_name(), [](std::error_code) {});
    ipc_manager_client.connect(ipc_manager_client.slots_[1].name, signal_b.full_name(), [](std::error_code) {});

    io_context.run_for(std::chrono::milliseconds(100));

    signal_a.send(true);
    io_context.run_for(std::chrono::milliseconds(100));

    expect(positioner.position() == -1 * mm) << positioner.position().numerical_value_;
  };

  return EXIT_SUCCESS;
}
