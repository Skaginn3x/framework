#include <mp-units/format.h>
#include <boost/ut.hpp>

#include <tfc/ipc/details/type_description.hpp>
#include <tfc/mocks/ipc.hpp>
#include <tfc/motor/positioner.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stubs/confman.hpp>
#include <tfc/testing/asio_clock.hpp>

using mp_units::si::unit_symbols::mm;
using mp_units::si::unit_symbols::ms;
namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::expect;
using ut::operator""_test;
using ut::operator|;
using ut::operator/;
using ut::operator>>;
using std::chrono_literals::operator""ms;
using std::chrono_literals::operator""ns;
static constexpr std::size_t buffer_len{ 10 };

using mock_bool_slot_t = tfc::ipc::mock_slot<tfc::ipc::details::type_bool, tfc::ipc_ruler::ipc_manager_client&>;
using mp_units::quantity;
using mp_units::si::unit_symbols::mm;

namespace compile_tests {

#ifdef __clang__

using tfc::motor::positioner::detail::make_between_callable;
// overflow tests
// numbers from 245 -> 255 and 0 -> 11
// is 244 between 245 and 11, nope
static_assert(!make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 244 }));
// is 12 between 245 and 11, nope
static_assert(!make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 12 }));
// is 246 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 246 }));
// is 1 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 1 }));
// is 1 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 0 }));
// is 255 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 255 }));
// is 245 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 245 }));
// is 11 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 })(std::uint8_t{ 11 }));

// normal incremental tests
// numbers from 11 -> 245
// is 11 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 11 }));
// is 245 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 245 }));
// is 12 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 12 }));
// is 244 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 244 }));
// is 0 between 11 and 245, nope
static_assert(!make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 0 }));
// is 1 between 11 and 245, nope
static_assert(!make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 1 }));
// is 255 between 11 and 245, nope
static_assert(!make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 255 }));
// is 10 between 11 and 245, nope
static_assert(!make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 10 }));
// is 246 between 11 and 245, nope
static_assert(!make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 })(std::uint8_t{ 246 }));

// underflow tests now we go backwards
// numbers from 11 -> 0 and 255 -> 245
// is 11 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 11 }));
// is 245 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 245 }));
// is 0 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 0 }));
// is 1 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 1 }));
// is 255 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 255 }));
// is 10 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 10 }));
// is 246 between 11 and 245, yes
static_assert(make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 246 }));
// is 12 between 11 and 245, nope
static_assert(!make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 12 }));
// is 244 between 11 and 245, nope
static_assert(!make_between_callable(std::uint8_t{ 11 }, std::uint8_t{ 245 }, false)(std::uint8_t{ 244 }));

// normal decrement tests
// numbers from 245 -> 11
// is 245 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 }, false)(std::uint8_t{ 245 }));
// is 11 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 }, false)(std::uint8_t{ 11 }));
// is 12 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 }, false)(std::uint8_t{ 12 }));
// is 244 between 245 and 11, yes
static_assert(make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 }, false)(std::uint8_t{ 244 }));
// is 0 between 245 and 11, nope
static_assert(!make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 }, false)(std::uint8_t{ 0 }));
// is 10 between 245 and 11, nope
static_assert(!make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 }, false)(std::uint8_t{ 10 }));
// is 246 between 245 and 11, nope
static_assert(!make_between_callable(std::uint8_t{ 245 }, std::uint8_t{ 11 }, false)(std::uint8_t{ 246 }));

#endif

}  // namespace compile_tests

#if (!(__GNUC__ && defined(DEBUG) && !__clang__))

struct test_instance {
  asio::io_context ctx{};
  std::shared_ptr<sdbusplus::asio::connection> dbus{ std::make_shared<sdbusplus::asio::connection>(ctx) };
  tfc::ipc_ruler::ipc_manager_client unused{ dbus };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"tachometer"> tachometer_test = [] {
  PRAGMA_CLANG_WARNING_POP
  // clang-format on
  using tachometer_t = tfc::motor::positioner::detail::tachometer<tfc::ipc_ruler::ipc_manager_client&, mock_bool_slot_t,
                                                                  tfc::testing::clock, buffer_len>;

  struct tachometer_test {
    test_instance inst{};
    // ability to populate, but will be moved into implementation
    std::function<tfc::motor::positioner::tick_signature_t> cb{ [](auto, auto, auto, auto) {} };
    tachometer_t tachometer{ inst.dbus, inst.unused, "name", std::move(cb) };
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
    bool called{};
    tachometer_test test{ .cb = [&called](std::int64_t new_value, auto, auto, auto) {
      expect(1 == new_value) << fmt::format("got {} expected {}", new_value, 1);
      called = true;
    } };
    for (std::size_t idx{}; idx < static_cast<std::int64_t>(buffer_len * 3); idx++) {
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

  ut::skip / "tachometer average deviation threshold reached"_test = [] {
    bool called{};
    bool do_expect_error{};
    tachometer_test test{ .cb = [&called, &do_expect_error](auto, auto, auto, tfc::motor::errors::err_enum err) {
      if (do_expect_error) {
        auto const expected{ tfc::motor::errors::err_enum::positioning_unstable };
        expect(err == expected) << fmt::format("expected: {}, got: {}\n", enum_name(expected), enum_name(err));
        called = true;
      }
    } };

    tfc::testing::clock::set_ticks(tfc::testing::clock::time_point{});
    // lets go 2 rounds to make sure the average is correct
    for (std::size_t idx{ 0 }; idx < buffer_len * 2; idx++) {
      tfc::testing::clock::time_point const later{ tfc::testing::clock::now() + 1ms };
      tfc::testing::clock::set_ticks(later);
      test.tachometer.update(false);  // for sake of completeness, even though it is unnecessary
      test.tachometer.update(true);
    }

    // let's get down to business
    do_expect_error = true;
    tfc::testing::clock::time_point const average_times_2{ tfc::testing::clock::now() +
                                                           test.tachometer.statistics().average() * 2 };
    tfc::testing::clock::set_ticks(average_times_2);
    test.tachometer.update(false);  // for sake of completeness, even though it is unnecessary
    test.tachometer.update(true);
    expect(called);
  };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"encoder"> enc_test = [] {
  PRAGMA_CLANG_WARNING_POP
  // clang-format on
  using encoder_t = tfc::motor::positioner::detail::encoder<tfc::ipc_ruler::ipc_manager_client&, mock_bool_slot_t,
                                                            tfc::testing::clock, buffer_len>;
  struct encoder_test {
    test_instance inst{};
    std::function<tfc::motor::positioner::tick_signature_t> cb{ [](std::int64_t, auto, auto, auto) {
    } };  // ability to populate, but will be moved into implementation
    encoder_t encoder{ inst.dbus, inst.unused, "name", std::move(cb) };
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

  ///	0   1   |   1   1   ->   +1
  "encoder: +1"_test = [] {
    encoder_test test{};

    test.encoder.first_tacho_update(true);
    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == -2) << test.encoder.position_;

    test.encoder.second_tacho_update(false);
    expect(test.encoder.position_ == -1);
  };

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

  {
    enum struct event_e : std::uint8_t { aoff = 0, boff, a, b };
    using enum event_e;
    "change direction"_test =
        [](std::vector<event_e> const& pulse_train) {
          encoder_test test{ .cb = [](auto, auto, auto, tfc::motor::errors::err_enum err) {
            expect(err == tfc::motor::errors::err_enum::success) << fmt::format("expected success, got {}\n", err);
          } };
          for (auto const& event : pulse_train) {
            switch (event) {
              case aoff:
                test.encoder.first_tacho_update(false);
                break;
              case boff:
                test.encoder.second_tacho_update(false);
                break;
              case a:
                test.encoder.first_tacho_update(true);
                break;
              case b:
                test.encoder.second_tacho_update(true);
                break;
            }
          }
          expect(test.encoder.position_ == 0) << fmt::format("expected 0, got {}\n", test.encoder.position_);
        } |
        // clang-format off
    std::vector<std::vector<event_e>>{
      // A _|‾‾|__|‾‾|_|‾‾|__|‾‾|_
      // B __|‾‾|__|‾‾‾‾‾|__|‾‾|__
      { a, b, aoff, boff, a, b, aoff, a, boff, aoff, b, a, boff, aoff },
      // A __|‾‾|__|‾‾‾‾‾|__|‾‾|_
      // B _|‾‾|__|‾‾|_|‾‾|__|‾‾|
      { b, a, boff, aoff, b, a, boff, b, aoff, boff, a, b, aoff, boff },
      // A _|‾‾|__|‾‾|_____|‾‾|__|‾‾|_
      // B |‾‾|__|‾‾|__|‾|__|‾‾|__|‾‾|
      { b, a, boff, aoff, b, a, boff, aoff, b, boff, a, b, aoff, boff, a, b, aoff, boff },
      // A |‾‾|__|‾‾|__|‾|__|‾‾|__|‾‾|
      // B _|‾‾|__|‾‾|_____|‾‾|__|‾‾|_
      { a, b, aoff, boff, a, b, aoff, boff, a, aoff, b, a, boff, aoff, b, a, boff, aoff },
      // A |‾‾|__|‾‾|___|‾‾|__|‾‾|_
      // B _|‾‾|__|‾‾|_|‾‾|__|‾‾|__
      { a, b, aoff, boff, a, b, aoff, boff, b, a, boff, aoff, b, a, boff, aoff },
      // A _|‾‾|__|‾‾|_|‾‾|__|‾‾|__
      // B |‾‾|__|‾‾|___|‾‾|__|‾‾|_
      { b, a, boff, aoff, b, a, boff, aoff, a, b, aoff, boff, a, b, aoff, boff },
    };
    // clang-format on
  }

  // we do expect that the encoder will receive event on first sensor than the second and so forth
  "missing event"_test = [](auto event) {
    bool called{};
    encoder_test test{ .cb = [&called, first_call = true](auto, auto, auto, tfc::motor::errors::err_enum err) mutable {
      if (first_call) {
        first_call = false;
        return;
      }
      called = true;
      auto const expected{ tfc::motor::errors::err_enum::positioning_missing_event };
      expect(err == expected) << fmt::format("expected: {}, got: {}\n", enum_name(expected), enum_name(err));
    } };

    test.encoder.update(1, true, false, event);
    test.encoder.update(1, true, false, event);

    expect(called);
  } | std::vector{ encoder_t::last_event_t::first, encoder_t::last_event_t::second };

  "NOT missing event if going forward and then backwards"_test = [](auto event) {
    bool called{};
    encoder_test test{ .cb = [&called, first_call = true](auto, auto, auto, tfc::motor::errors::err_enum err) mutable {
      if (first_call) {
        first_call = false;
        return;
      }
      called = true;
      auto const expected{ tfc::motor::errors::err_enum::success };
      expect(err == expected) << fmt::format("expected: {}, got: {}\n", enum_name(expected), enum_name(err));
    } };

    test.encoder.update(1, true, false, event);
    test.encoder.update(-1, true, false, event);

    expect(called);
  } | std::vector{ encoder_t::last_event_t::first, encoder_t::last_event_t::second };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"notifications"> notify_tests = [] {
  PRAGMA_CLANG_WARNING_POP
  // clang-format on
  static constexpr auto unit{ mp_units::si::metre };
  static constexpr auto reference{ mp_units::si::nano<unit> };
  struct notification_test {
    using positioner_t = tfc::motor::positioner::
        positioner<unit, tfc::ipc_ruler::ipc_manager_client&, tfc::confman::stub_config, mock_bool_slot_t>;
    using position_t = positioner_t::absolute_position_t;
    using home_travel_t = tfc::confman::observable<std::optional<positioner_t::absolute_position_t>>;
    using tachometer_config_t = tfc::motor::positioner::tachometer_config<reference>;
    test_instance inst{};
    positioner_t::config_t config{};  // to be moved to implementation
    std::function<void(bool)> home_cb{ [](bool) {} };
    std::function<void(bool)> positive_limit_cb{ [](bool) {} };
    std::function<void(bool)> negative_limit_cb{ [](bool) {} };
    positioner_t positioner{
      inst.dbus,        inst.unused, "name", std::move(home_cb), std::move(positive_limit_cb), std::move(negative_limit_cb),
      std::move(config)
    };
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
    auto constexpr displacement{ 100 * mm };
    notification_test::positioner_t::config_t config{};
    using tachometer_config_t = tfc::motor::positioner::tachometer_config<mp_units::si::nano<mp_units::si::metre>>;
    tachometer_config_t tachometer_config{};
    tachometer_config.displacement_per_increment = displacement;
    config.mode = tachometer_config;

    notification_test test{ .config = config };
    test.positioner.tick(1, {}, {}, {});
    expect(test.positioner.position() == displacement)
        << fmt::format("expected: {}, got: {}\n", displacement, test.positioner.position());
  };

  "velocity per tick"_test = [] {
    auto constexpr displacement{ 100 * mm };
    notification_test::positioner_t::config_t config{};
    notification_test::tachometer_config_t tachometer_config{};
    tachometer_config.displacement_per_increment = displacement;
    config.mode = tachometer_config;

    notification_test test{ .config = config };
    auto const tick_duration{ 1ms };
    test.positioner.tick(1, tick_duration, {}, {});
    auto expected_velocity{ displacement / (tick_duration.count() * mp_units::si::milli<mp_units::si::second>)};
    expect(expected_velocity == 100 * mm / ms) << fmt::format("expected: {}, got: {}\n", 100 * mm / ms, expected_velocity);
    expect(test.positioner.velocity() == expected_velocity)
        << fmt::format("expected: {}, got: {}\n", expected_velocity, test.positioner.velocity());
  };

  ut::skip / "standard deviation error"_test = [] {
    auto constexpr stddev{ 1ms };
    notification_test::positioner_t::config_t config{};
    notification_test::tachometer_config_t tachometer_config{};
    tachometer_config.standard_deviation_threshold = stddev;
    config.mode = tachometer_config;

    notification_test test{ .config = config };
    test.positioner.tick(1, {}, stddev, {});
    expect(test.positioner.error() == tfc::motor::errors::err_enum::positioning_unstable);
  };

  "homing required not normally"_test = [] {
    using enum tfc::motor::errors::err_enum;
    notification_test test{};
    expect(test.positioner.error() == success);
  };

  "homing required on construction if homing travel is configured"_test = [] {
    using enum tfc::motor::errors::err_enum;
    notification_test test{ .config = { .needs_homing_after = notification_test::home_travel_t{ 1 * mm } } };
    expect(test.positioner.error() == motor_missing_home_reference);
  };

  "homing not required if homed"_test = [] {
    using enum tfc::motor::errors::err_enum;
    notification_test test{ .config = { .needs_homing_after = notification_test::home_travel_t{ 1 * mm } } };
    test.positioner.home();
    expect(test.positioner.error() == success);
  };

  "homing required if homed and exceeded config param"_test = [] {
    using enum tfc::motor::errors::err_enum;
    notification_test test{ .config = { .needs_homing_after = notification_test::home_travel_t{ 2 * mm } } };
    test.positioner.home();
    test.positioner.increment_position(1 * mm);
    test.positioner.increment_position(-1 * mm);
    expect(test.positioner.error() == motor_missing_home_reference);
  };

  struct flow_test {
    notification_test::position_t value{};
    std::int64_t ticks{};
  };
  "under or overflow"_test =
      [](flow_test const& data) {
        notification_test test{};
        test.positioner.increment_position(data.value);
        bool called{};
        test.positioner.notify_after(data.ticks * 2 * mm, [&called](std::error_code err) {
          expect(!err) << err.message();
          called = true;
        });
        test.positioner.increment_position(data.ticks * mm);
        test.inst.ctx.run_for(1ms);
        expect(!called);
        test.positioner.increment_position(data.ticks * mm);
        test.inst.ctx.run_for(1ms);
        expect(called);
      } |
      std::vector{ flow_test{ .value = notification_test::position_t::max(), .ticks = 1 },
                   flow_test{ .value = notification_test::position_t::min(), .ticks = -1 } };

  "homing velocity enabled call home callback"_test = [] {
    using tfc::confman::observable;
    using tfc::motor::positioner::speedratio_t;
    bool called{};
    notification_test test{ .config = { .homing_travel_speed =
                                            observable<std::optional<speedratio_t>>{ 2 * mp_units::percent } },
                            .home_cb = [&called](bool new_v) { called = new_v; } };
    expect(test.positioner.homing_sensor().has_value() >> ut::fatal);
    test.positioner.homing_sensor()->callback(true);
    expect(called);
  };

  "positive limit switch enabled call callback"_test = [] {
    using tfc::confman::observable;
    using tfc::motor::positioner::speedratio_t;
    bool called{};
    notification_test test{ .config = { .homing_travel_speed =
                                            observable<std::optional<speedratio_t>>{ 2 * mp_units::percent } },
                            .positive_limit_cb = [&called](bool new_v) { called = new_v; } };
    expect(test.positioner.positive_limit_switch().has_value() >> ut::fatal);
    test.positioner.positive_limit_switch()->callback(true);
    expect(called);
  };

  "negative limit switch enabled call callback"_test = [] {
    using tfc::confman::observable;
    using tfc::motor::positioner::speedratio_t;
    bool called{};
    notification_test test{ .config = { .homing_travel_speed =
                                            observable<std::optional<speedratio_t>>{ 2 * mp_units::percent } },
                            .negative_limit_cb = [&called](bool new_v) { called = new_v; } };
    expect(test.positioner.negative_limit_switch().has_value() >> ut::fatal);
    test.positioner.negative_limit_switch()->callback(true);
    expect(called);
  };
};
#endif

int main(int argc, char** argv) {
  using tfc::motor::positioner::detail::circular_buffer;
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
