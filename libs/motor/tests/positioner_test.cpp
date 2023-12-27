#include <mp-units/format.h>
#include <boost/ut.hpp>

#include <tfc/ipc/details/type_description.hpp>
#include <tfc/mocks/ipc.hpp>
#include <tfc/motor/positioner.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/testing/asio_clock.hpp>

using namespace mp_units::si::unit_symbols;
namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::expect;
using ut::operator""_test;
using ut::operator|;
using ut::operator/;
using std::chrono_literals::operator""ms;
using std::chrono_literals::operator""ns;
static constexpr std::size_t buffer_len{ 10 };

using mock_bool_slot_t = tfc::ipc::mock_slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client&>;
using mp_units::quantity;
using mp_units::si::unit_symbols::mm;

#if (!(__GNUC__ && defined(DEBUG) && !__clang__))

struct test_instance {
  asio::io_context ctx{};
  tfc::ipc_ruler::ipc_manager_client client{ ctx };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"tachometer"> tachometer_test = [] {
  PRAGMA_CLANG_WARNING_POP
  // clang-format on
  using tachometer_t = tfc::motor::detail::tachometer<mock_bool_slot_t, tfc::testing::clock, buffer_len>;

  struct tachometer_test {
    test_instance inst{};
    // ability to populate, but will be moved into implementation
    std::function<tfc::motor::tick_signature_t> cb{ [](auto, auto, auto, auto) {} };
    tachometer_t tachometer{ inst.ctx, inst.client, "name", std::move(cb) };
  };

  "tachometer with single sensor updates internal state"_test = [] {
    tachometer_test test{};
    for (std::int64_t idx{ 0 }; idx < static_cast<std::int64_t>(buffer_len * 3); idx++) {
      expect(test.tachometer.position_ == idx);
      test.tachometer.update(true);
      test.tachometer.update(false);
    }
  };

  "tachometer with single sensor calls owner"_test = [] {
    std::int64_t idx{ 1 };
    bool called{};
    tachometer_test test{ .cb = [&idx, &called](std::int64_t new_value, auto, auto, auto) {
      expect(idx == new_value) << fmt::format("got {} expected {}", new_value, idx);
      called = true;
    } };
    for (; idx < static_cast<std::int64_t>(buffer_len * 3); idx++) {
      test.tachometer.update(true);
      test.tachometer.update(false);
    }
  };

  "tachometer with single sensor stores new values to buffer on rising edge"_test = [] {
    tachometer_test test{};
    tfc::testing::clock::set_ticks(tfc::testing::clock::time_point{});
    for (std::int64_t idx{ 0 }; idx < static_cast<std::int64_t>(buffer_len * 3); idx++) {
      tfc::testing::clock::time_point const later{ tfc::testing::clock::now() + 1ms };
      tfc::testing::clock::set_ticks(later);
      test.tachometer.update(false);
      test.tachometer.update(true);
      expect(test.tachometer.statistics().buffer().front().time_point == later);
    }
  };

  "tachometer average"_test = [] {
    tachometer_test test{};
    tfc::testing::clock::set_ticks(tfc::testing::clock::time_point{});
    for (std::size_t idx{ 0 }; idx < buffer_len; idx++) {
      tfc::testing::clock::time_point const later{ tfc::testing::clock::now() + std::chrono::milliseconds{ idx } };
      tfc::testing::clock::set_ticks(later);
      test.tachometer.update(false);  // for sake of completeness, even though it is unnecessary
      test.tachometer.update(true);
    }
    std::chrono::nanoseconds average{ std::chrono::microseconds{ 4500 } };
    expect(test.tachometer.statistics().average() == average)
        << fmt::format("expected average: {}, got average: {}\n", average, test.tachometer.statistics().average());
  };

  "tachometer variance"_test = [] {
    tachometer_test test{};
    tfc::testing::clock::set_ticks(tfc::testing::clock::time_point{});
    // lets go 2 rounds to make sure the average is correct
    for (std::size_t idx{ 0 }; idx < buffer_len * 2; idx++) {
      tfc::testing::clock::time_point const later{ tfc::testing::clock::now() + 1ms };
      tfc::testing::clock::set_ticks(later);
      test.tachometer.update(false);  // for sake of completeness, even though it is unnecessary
      test.tachometer.update(true);
    }
    expect(test.tachometer.statistics().stddev() == 0ms)
        << fmt::format("expected stddev: {}, got stddev: {}\n", 0ms, test.tachometer.statistics().stddev());
  };

  struct data_t {
    std::chrono::nanoseconds time_between_teeth{};
    std::chrono::nanoseconds stddev{};
  };
  "tachometer one missing tooth"_test =
      [](auto& data) {
        // let's say we have 10 teeth
        //                 x  0
        //               x 9 x  1
        //             x 8     x  2
        //             x 7     x  3
        //               x 6 x  4 <- missing tooth
        //                 x  5
        ut::test(fmt::format("interval {}", data.time_between_teeth)) = [&data] {
          tachometer_test test{};
          tfc::testing::clock::set_ticks(tfc::testing::clock::time_point{});
          for (std::size_t idx{ 0 }; idx < buffer_len * 2; idx++) {
            tfc::testing::clock::time_point const later{ tfc::testing::clock::now() + data.time_between_teeth };
            tfc::testing::clock::set_ticks(later);
            if (idx == 4 || idx == 14) {
              continue;
            }
            test.tachometer.update(false);  // for sake of completeness, even though it is unnecessary
            test.tachometer.update(true);
          }
          expect(test.tachometer.statistics().stddev() == data.stddev)
              << fmt::format("expected stddev: {}, got stddev: {}\n", data.stddev, test.tachometer.statistics().stddev());
        };
      } |
      // todo verify by hand the calculation is correct
      std::vector{ data_t{ .time_between_teeth = 1ms, .stddev = 303315ns },
                   data_t{ .time_between_teeth = 3ms, .stddev = 909945ns },
                   data_t{ .time_between_teeth = 7ms, .stddev = 2123205ns },
                   data_t{ .time_between_teeth = 17ms, .stddev = 5156355ns } };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"encoder"> enc_test = [] {
  PRAGMA_CLANG_WARNING_POP
  // clang-format on
  using encoder_t = tfc::motor::detail::encoder<mock_bool_slot_t, tfc::testing::clock, buffer_len>;
  struct encoder_test {
    test_instance inst{};
    std::function<tfc::motor::tick_signature_t> cb{ [](std::int64_t, auto, auto, auto) {
    } };  // ability to populate, but will be moved into implementation
    encoder_t encoder{ inst.ctx, inst.client, "name", std::move(cb) };
  };

  "encoder: 0"_test = [] {
    encoder_test test{};

    expect(test.encoder.position_ == 0);

    test.encoder.first_tacho_update(false);
    test.encoder.second_tacho_update(false);

    expect(test.encoder.position_ == 0);
  };

  //	0   1   |   0   0   ->  -1
  "encoder: -1"_test = [] {
    bool called{};
    encoder_test test{ .cb = [&called](std::int64_t pos, auto, auto, auto) {
      expect(pos == -1) << fmt::format("expected -1, got {}\n", pos);
      called = true;
    } };
    test.encoder.first_tacho_update(true);
    expect(test.encoder.position_ == -1) << fmt::format("expected -1, got {}\n", test.encoder.position_);
    expect(called);
  };

  //	1   0   |   0   0   ->   +1
  "encoder: +1"_test = [] {
    bool called{};
    encoder_test test{ .cb = [&called](std::int64_t pos, auto, auto, auto) {
      expect(pos == 1) << fmt::format("expected 1, got {}\n", pos);
      called = true;
    } };
    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == 1);
    expect(called);
  };

  ///	0   0   |   0   1   ->   +1
  "encoder: +1"_test = [] {
    encoder_test test{};

    test.encoder.first_tacho_update(true);
    expect(test.encoder.position_ == -1);

    test.encoder.first_tacho_update(false);
    expect(test.encoder.position_ == 0);
  };

  ///	0   0   |   1   0   ->   -1
  "encoder: -1"_test = [] {
    encoder_test test{};

    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == 1);

    test.encoder.second_tacho_update(false);
    expect(test.encoder.position_ == 0);
  };

  ///	0   1   |   0   1   ->   no movement
  // "encoder: 0"_test = [] {
  //   encoder_test test{};

  //   test.encoder.first_tacho_update(true);
  //   expect(test.encoder.position_ == -1);

  //   test.encoder.first_tacho_update(true);
  //   expect(test.encoder.position_ == -1);
  // };

  ///	0   1   |   1   1   ->   +1
  "encoder: +1"_test = [] {
    encoder_test test{};

    test.encoder.first_tacho_update(true);
    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == -2) << test.encoder.position_;

    test.encoder.second_tacho_update(false);
    expect(test.encoder.position_ == -1);
  };

  ///	1   0   |   1   0   ->   no movement
  // "encoder: 0"_test = [] {
  //   encoder_test test{};

  //   test.encoder.second_tacho_update(true);
  //   expect(test.encoder.position_ == 1);

  //   test.encoder.second_tacho_update(true);
  //   expect(test.encoder.position_ == 1);
  // };

  ///	1   0   |   1   1   ->   -1
  "encoder: -1"_test = [] {
    encoder_test test{};

    test.encoder.first_tacho_update(true);
    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == -2) << test.encoder.position_;

    test.encoder.first_tacho_update(false);
    expect(test.encoder.position_ == -3);
  };

  ///   1   1   |   0   1   ->   -1
  "encoder: -1"_test = [] {
    encoder_test test{};

    test.encoder.first_tacho_update(true);
    expect(test.encoder.position_ == -1) << test.encoder.position_;

    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == -2) << test.encoder.position_;
  };

  ///   1   1   |   1   0   ->   +1
  "encoder: +1"_test = [] {
    encoder_test test{};

    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == 1) << test.encoder.position_;

    test.encoder.first_tacho_update(true);
    expect(test.encoder.position_ == 2) << test.encoder.position_;
  };

  ///   1   1   |   1   1   ->   no movement
  // "encoder: 0"_test = [] {
  //   encoder_test test{};

  //   test.encoder.first_tacho_update(true);
  //   test.encoder.second_tacho_update(true);
  //   expect(test.encoder.position_ == -2) << test.encoder.position_;

  //   test.encoder.first_tacho_update(true);
  //   test.encoder.second_tacho_update(true);
  //   expect(test.encoder.position_ == -2) << test.encoder.position_;
  // };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"notifications"> notify_tests = [] {
  PRAGMA_CLANG_WARNING_POP
  // clang-format on
  struct notification_test {
    using positioner_t = tfc::motor::positioner<tfc::motor::position_t, tfc::confman::stub_config>;
    test_instance inst{};
    positioner_t::config config{};  // to be moved to implementation
    positioner_t positioner{ inst.ctx, inst.client, "name", std::move(config) };
  };
  using mp_units::si::unit_symbols::mm;

  "notify_at sort moves items, the items should be intact"_test = [] {
    notification_test test{};
    bool called{};
    test.positioner.notify_at(2 * mm, [](std::error_code) { expect(false); });
    test.positioner.notify_at(1 * mm, [&called](std::error_code err) {
      expect(!err) << err.message();
      called = true;
    });
    test.positioner.increment_position(1 * mm);
    test.inst.ctx.run_for(1ms);
    expect(called);
  };

  "notify_at calls callback after position is reached"_test = [] {
    notification_test test{};
    bool called{};
    test.positioner.notify_at(11 * mm, [](std::error_code) { expect(false); });
    test.positioner.notify_at(2 * mm, [&called](std::error_code err) {
      expect(!err) << err.message();
      called = true;
    });
    // test that sorting works
    test.positioner.notify_at(10 * mm, [](std::error_code) { expect(false); });
    test.positioner.increment_position(1 * mm);
    test.inst.ctx.run_for(1ms);
    expect(!called);

    test.positioner.increment_position(1 * mm);
    test.inst.ctx.run_for(1ms);
    expect(called);

    // test that callback is not called again
    called = false;
    test.positioner.increment_position(-1 * mm);
    test.positioner.increment_position(1 * mm);
    test.inst.ctx.run_for(1ms);
    expect(!called);
  };

  "notify_at go forward then backwards"_test = [] {
    notification_test test{};
    test.positioner.increment_position(100 * mm);
    bool called_80{};
    test.positioner.notify_at(80 * mm, [&](std::error_code) { called_80 = true; });
    test.positioner.notify_at(101 * mm, [](std::error_code) { expect(false); });
    test.positioner.notify_at(102 * mm, [](std::error_code) { expect(false); });
    bool called{};
    test.positioner.notify_after(1000 * mm, [&](std::error_code) { expect(false); });
    bool called_neg1000{};
    test.positioner.notify_after(-1000 * mm, [&](std::error_code) { called_neg1000 = true; });
    test.positioner.notify_at(90 * mm, [&called](std::error_code err) {
      expect(!err) << err.message();
      called = true;
    });
    test.positioner.increment_position(-11000 * mm);
    test.inst.ctx.run_for(1ms);
    expect(called);
    expect(called_80);
    expect(called_neg1000);
  };

  "notify_at go backwards notification on same place"_test = [] {
    notification_test test{};
    test.positioner.notify_at(-20 * mm, [](std::error_code) { expect(false); });
    bool called1{};
    test.positioner.notify_at(-10 * mm, [&called1](std::error_code err) {
      expect(!err) << err.message();
      called1 = true;
    });
    bool called2{};
    test.positioner.notify_at(-10 * mm, [&called2](std::error_code err) {
      expect(!err) << err.message();
      called2 = true;
    });
    test.positioner.increment_position(-10 * mm);
    test.inst.ctx.run_for(1ms);
    expect(called1);
    expect(called2);
  };

  "notify_at the current position then increment position"_test = [](auto increment) {
    // This is an edge case and I say that it should notify
    // todo discuss if needed
    notification_test test{};
    test.positioner.increment_position(-100 * mm);
    test.inst.ctx.run_for(1ms);
    bool called{};
    test.positioner.notify_at(-100 * mm, [&called](std::error_code err) {
      expect(!err);
      called = true;
    });
    test.positioner.increment_position(increment);
    test.inst.ctx.run_for(1ms);
    expect(called);
  } | std::vector{ 1 * mm, -1 * mm };  // it can go either forward or backwards from current position

  "position per tick"_test = [] {
    auto const displacement{ 100 * mm };
    notification_test test{ .config = { .displacement_per_increment = displacement } };
    test.positioner.tick(1, {}, {}, {});
    expect(test.positioner.position() == displacement)
        << fmt::format("expected: {}, got: {}\n", displacement, test.positioner.position());
  };

  "velocity per tick"_test = [] {
    auto const displacement{ 100 * mm };
    notification_test test{ .config = { .displacement_per_increment = displacement } };
    auto const tick_duration{ 1ms };
    test.positioner.tick(1, tick_duration, {}, {});
    auto expected_velocity{ displacement / (tick_duration.count() * mp_units::si::milli<mp_units::si::second>)};
    expect(expected_velocity == 100 * mm / ms) << fmt::format("expected: {}, got: {}\n", 100 * mm / ms, expected_velocity);
    expect(test.positioner.velocity() == expected_velocity)
        << fmt::format("expected: {}, got: {}\n", expected_velocity, test.positioner.velocity());
  };

  "standard deviation error"_test = [] {
    auto stddev{ 1ms };
    notification_test test{ .config = { .standard_deviation_threshold = stddev } };
    test.positioner.tick(1, {}, stddev, {});
    expect(test.positioner.error() == tfc::motor::position_error_code_e::unstable);
  };
};
#endif

int main(int argc, char** argv) {
  using tfc::motor::detail::circular_buffer;
  tfc::base::init(argc, argv);

  "circular_buffer_test moves pointer front when inserted to last item"_test = [] {
    static constexpr std::size_t len{ 10 };
    circular_buffer<int, 10> buff{};
    for (std::size_t i = 0; i < len + len; ++i) {
      // try two rounds
      buff.emplace(i);
    }
    expect(buff.front() == len + len - 1) << buff.front();
    expect(buff.back() == len) << buff.back();  // back is the oldest item
  };

  "circular_buffer_test returns oldest item when emplacing new one"_test = [] {
    static constexpr std::size_t len{ 3 };
    circular_buffer<int, len> buff{};
    expect(buff.emplace(1) == 0);
    expect(buff.emplace(2) == 0);
    expect(buff.emplace(3) == 0);
    expect(buff.emplace(4) == 1);
    expect(buff.emplace(5) == 2);
    expect(buff.emplace(6) == 3);
    expect(buff.emplace(7) == 4);
  };

  return EXIT_SUCCESS;
}
