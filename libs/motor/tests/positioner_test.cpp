#include <boost/ut.hpp>
#include <tfc/ipc/details/dbus_client_iface_mock.hpp>
#include <tfc/motors/positioner.hpp>
#include <tfc/progbase.hpp>

using namespace mp_units::si::unit_symbols;
namespace asio = boost::asio;

using mp_units::quantity;
using mp_units::si::unit_symbols::mm;

class double_tacho_config_mock {
public:
  double_tacho_config_mock(asio::io_context&, std::string_view) {}
  tfc::motor::detail::tacho_config tacho{ tfc::motor::detail::tacho_config::two_tacho };
  mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t> displacement_per_pulse{ 1 * mm };
  // Overload the dereference operator
  auto operator->() -> const double_tacho_config_mock* { return this; }
};

int main(int argc, char** argv) {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  using tfc::motor::detail::circular_buffer;
  tfc::base::init(argc, argv);

  "circular_buffer_test moves pointer front when inserted to last item"_test = [] {
    static constexpr std::size_t len{ 10 };
    circular_buffer<int, 10> buff{};
    for (std::size_t i = 0; i < len + len; ++i) {  // try two rounds
      buff.emplace(i);
    }
    expect(buff.front() == len + len - 1) << buff.front();
  };

  std::function<void(std::int64_t)> const empty_callback{ [](std::int64_t) {} };

  "tachometer with single sensor"_test = [&empty_callback] {
    tfc::motor::detail::single_tachometer<> tacho{ empty_callback };

    expect(tacho.position_ == 0);
    tacho.first_tacho_update(true);
    expect(tacho.position_ == 1);
    tacho.first_tacho_update(true);
    expect(tacho.position_ == 2);
    tacho.first_tacho_update(true);
    expect(tacho.position_ == 3);
  };

  "tachometer: 0"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    expect(tacho.position_ == 0);

    tacho.first_tacho_update(false);
    tacho.second_tacho_update(false);

    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   0   ->  -1
  "tachometer: -1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };
    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);
  };

  ///	1   0   |   0   0   ->   +1
  "tachometer: +1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };
    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);
  };

  ///	0   0   |   0   1   ->   +1
  "tachometer: +1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);

    tacho.first_tacho_update(false);
    expect(tacho.position_ == 0);
  };

  ///	0   0   |   1   0   ->   -1
  "tachometer: -1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);

    tacho.second_tacho_update(false);
    expect(tacho.position_ == 0);
  };

  ///	0   1   |   0   1   ->   no movement
  "tachometer: 0"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1);
  };

  ///	0   1   |   1   1   ->   +1
  "tachometer: +1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.second_tacho_update(false);
    expect(tacho.position_ == -1);
  };

  ///	1   0   |   1   0   ->   no movement
  "tachometer: 0"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1);
  };

  ///	1   0   |   1   1   ->   -1
  "tachometer: -1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(false);
    expect(tacho.position_ == -3);
  };

  ///   1   1   |   0   1   ->   -1
  "tachometer: -1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.first_tacho_update(true);
    expect(tacho.position_ == -1) << tacho.position_;

    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;
  };

  ///   1   1   |   1   0   ->   +1
  "tachometer: +1"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.second_tacho_update(true);
    expect(tacho.position_ == 1) << tacho.position_;

    tacho.first_tacho_update(true);
    expect(tacho.position_ == 2) << tacho.position_;
  };

  ///   1   1   |   1   1   ->   no movement
  "tachometer: 0"_test = [&empty_callback] {
    tfc::motor::detail::double_tachometer<> tacho{ empty_callback };

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;

    tacho.first_tacho_update(true);
    tacho.second_tacho_update(true);
    expect(tacho.position_ == -2) << tacho.position_;
  };

  "test notifications and single tacho"_test = [] {
    using mp_units::si::unit_symbols::mm;
    using mp_units::quantity;

    boost::asio::io_context io_context{};
    tfc::ipc_ruler::ipc_manager_client_mock ipc_manager_client{ io_context };
    tfc::motor::positioner<mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t>,
                           tfc::ipc_ruler::ipc_manager_client_mock&, double_tacho_config_mock>
        positioner{ io_context, ipc_manager_client, "positioner_name" };

    std::int64_t callback_counter = 0;

    mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t> const position_1mm{ 1 * mm };
    mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t> const position_2mm{ 2 * mm };
    mp_units::quantity<mp_units::si::milli<mp_units::si::metre>, std::int64_t> const position_3mm{ 3 * mm };

    std::function<void()> const callback{ [&callback_counter]() { callback_counter++; } };

    positioner.notify_after(position_3mm, callback);
    positioner.notify_after(position_2mm, callback);
    positioner.notify_after(position_1mm, callback);

    expect(positioner.position() == 0 * mm);

    using mock_bool_signal = tfc::ipc::signal<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client_mock&>;
    using std::chrono::milliseconds;

    mock_bool_signal signal_a{ io_context, ipc_manager_client, "tacho_signal_a" };
    io_context.run_for(milliseconds(5));

    mock_bool_signal signal_b{ io_context, ipc_manager_client, "tacho_signal_b" };
    io_context.run_for(milliseconds(5));

    ipc_manager_client.connect(ipc_manager_client.slots_[0].name, signal_a.full_name(), [](std::error_code) {});
    ipc_manager_client.connect(ipc_manager_client.slots_[1].name, signal_b.full_name(), [](std::error_code) {});

    io_context.run_for(milliseconds(5));

    signal_b.send(true);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 1 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 1);

    signal_a.send(true);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 2 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 2);

    signal_b.send(false);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 3 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 3);

    signal_a.send(false);
    io_context.run_for(milliseconds(5));

    expect(positioner.position() == 4 * mm) << positioner.position().numerical_value_;
    expect(callback_counter == 3);
  };

  return EXIT_SUCCESS;
}
