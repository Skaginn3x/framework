#include <boost/ut.hpp>
#include <tfc/motors/positioner.hpp>

#include <tfc/ipc/details/dbus_client_iface_mock.hpp>

using mp_units::quantity;
using mp_units::si::unit_symbols::mm;
using tfc::motor::detail::tacho_config_e;

namespace asio = boost::asio;

class single_tacho_config_mock {
public:
  single_tacho_config_mock(asio::io_context&, std::string_view) {}

  tacho_config_e tacho{ tacho_config_e::one_tacho };
  quantity<mm> displacement_per_pulse{ 1 * mm };

  // Overload the dereference operator
  auto operator->() -> const single_tacho_config_mock* { return this; }
};

class double_tacho_config_mock {
public:
  double_tacho_config_mock(asio::io_context&, std::string_view) {}

  tacho_config_e tacho{ tacho_config_e::two_tacho };
  quantity<mm> displacement_per_pulse{ 1 * mm };

  // Overload the dereference operator
  auto operator->() -> const double_tacho_config_mock* { return this; }
};

auto main(int argc, char** argv) -> int {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  using mp_units::si::unit_symbols::mm;
  using std::chrono::milliseconds;
  using tfc::motor::detail::double_tachometer;
  using single_tacho_positioner = tfc::motor::positioner<tfc::ipc_ruler::ipc_manager_client_mock&, single_tacho_config_mock>;
  using double_tacho_positioner = tfc::motor::positioner<tfc::ipc_ruler::ipc_manager_client_mock&, double_tacho_config_mock>;
  using mock_bool_signal = tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&>;

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
    double_tachometer tacho{};

    expect(tacho.position_ == 0);

    tacho.first_tacho_update(false);
    tacho.second_tacho_update(false);

    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   0   ->  -1
  "tachometer: -1"_test = [] {
    double_tachometer tacho{};
    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);
  };

  ///	1   0   |   0   0   ->   +1
  "tachometer: +1"_test = [] {
    double_tachometer tacho{};
    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);
  };

  ///	0   0   |   0   1   ->   +1
  "tachometer: +1"_test = [] {
    double_tachometer tacho{};

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);

    tacho.first_tacho_update(false);
    expect(tacho.position_ == 0);
  };

  ///	0   0   |   1   0   ->   -1
  "tachometer: -1"_test = [] {
    double_tachometer tacho{};

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);

    tacho.second_tacho_update(false);
    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   1   ->   no movement
  "tachometer: 0"_test = [] {
    double_tachometer tacho{};

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);
  };

  ///	0   1   |   1   1   ->   +1
  "tachometer: +1"_test = [] {
    double_tachometer tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.second_tacho_update(false);
    expect(tacho.position_ == -1);
  };

  ///	1   0   |   1   0   ->   no movement
  "tachometer: 0"_test = [] {
    double_tachometer tacho{};

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);
  };

  ///	1   0   |   1   1   ->   -1
  "tachometer: -1"_test = [] {
    double_tachometer tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(false);
    expect(tacho.position_ == -3);
  };

  ///   1   1   |   0   1   ->   -1
  "tachometer: -1"_test = [] {
    double_tachometer tacho{};

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1) << tacho.position_;

    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;
  };

  ///   1   1   |   1   0   ->   +1
  "tachometer: +1"_test = [] {
    double_tachometer tacho{};

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1) << tacho.position_;

    tacho.first_tacho_update(true);
    expect(tacho.position_ == 2) << tacho.position_;
  };

  ///   1   1   |   1   1   ->   no movement
  "tachometer: 0"_test = [] {
    double_tachometer tacho{};

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;
  };

  "test callback function"_test = [] {
    std::int64_t callback_counter = 0;

    std::function<void(std::int64_t)> const callback{ [&callback_counter](std::int64_t) { callback_counter++; } };

    double_tachometer tacho{ callback };

    tacho.first_tacho_update(true);
    expect(callback_counter == 1);

    tacho.second_tacho_update(true);
    expect(callback_counter == 2);
  };

  "test positioner"_test = [] {
    asio::io_context io_context{};
    tfc::ipc_ruler::ipc_manager_client_mock ipc_manager_client{ io_context };
    single_tacho_positioner const positioner{ io_context, ipc_manager_client, "positioner_name" };

    expect(positioner.position() == 0 * mm);

    mock_bool_signal signal{ io_context, ipc_manager_client, "tacho_signal" };
    ipc_manager_client.connect(ipc_manager_client.slots_[0].name, signal.full_name(), [](std::error_code) {});
    io_context.run_for(milliseconds(5));

    signal.send(true);
    io_context.run_for(milliseconds(5));
    expect(positioner.position() == 1 * mm) << positioner.position().numerical_value_;

    signal.send(false);
    io_context.run_for(milliseconds(5));
    expect(positioner.position() == 1 * mm) << positioner.position().numerical_value_;

    signal.send(true);
    io_context.run_for(milliseconds(5));
    expect(positioner.position() == 2 * mm) << positioner.position().numerical_value_;
  };

  "test notifications and single tacho"_test = [] {
    using mp_units::si::unit_symbols::mm;
    using mp_units::quantity;

    asio::io_context io_context{};
    tfc::ipc_ruler::ipc_manager_client_mock ipc_manager_client{ io_context };

    single_tacho_positioner positioner{ io_context, ipc_manager_client, "positioner_name" };

    std::int64_t callback_counter = 0;

    std::function<void(quantity<mm>)> callback{ [&callback_counter](quantity<mm>) { callback_counter++; } };

    quantity<mm> const position_1mm{ 1 * mm };
    quantity<mm> const position_2mm{ 2 * mm };
    quantity<mm> const position_3mm{ 3 * mm };

    positioner.notify_after(position_3mm, callback);
    positioner.notify_after(position_2mm, callback);
    positioner.notify_after(position_1mm, callback);

    expect(positioner.position() == 0 * mm);

    mock_bool_signal signal{ io_context, ipc_manager_client, "tacho_signal" };
    ipc_manager_client.connect(ipc_manager_client.slots_[0].name, signal.full_name(), [](std::error_code) {});
    io_context.run_for(milliseconds(5));

    signal.send(true);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 1 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 1);

    signal.send(false);
    io_context.run_for(milliseconds(5));

    signal.send(true);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 2 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 2);

    signal.send(false);
    io_context.run_for(milliseconds(5));

    signal.send(true);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 3 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 3);
  };

  "test double tacho"_test = [] {
    using mp_units::si::unit_symbols::mm;
    using mp_units::quantity;

    asio::io_context io_context{};
    tfc::ipc_ruler::ipc_manager_client_mock ipc_manager_client{ io_context };

    double_tacho_positioner const positioner{ io_context, ipc_manager_client, "positioner_name" };

    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 0 * mm);

    mock_bool_signal signal_a{ io_context, ipc_manager_client, "tacho_signal_a" };
    io_context.run_for(milliseconds(5));

    mock_bool_signal const signal_b{ io_context, ipc_manager_client, "tacho_signal_b" };
    io_context.run_for(milliseconds(5));

    ipc_manager_client.connect(ipc_manager_client.slots_[0].name, signal_a.full_name(), [](std::error_code) {});
    ipc_manager_client.connect(ipc_manager_client.slots_[1].name, signal_b.full_name(), [](std::error_code) {});

    io_context.run_for(milliseconds(5));

    signal_a.send(true);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == -1 * mm) << positioner.position().numerical_value_;
  };

  return EXIT_SUCCESS;
}
