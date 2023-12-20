#include <boost/ut.hpp>

#include <tfc/ipc/details/type_description.hpp>
#include <tfc/mocks/ipc.hpp>
#include <tfc/motors/positioner.hpp>
#include <tfc/progbase.hpp>
#include <tfc/testing/asio_clock.hpp>

namespace compile_tests {

using update_params = tfc::motor::detail::encoder<>::update_params;
static_assert(sizeof(update_params) == 1, "update_params should be 1 byte");
static_assert(update_params{} == 0U);
static_assert(update_params{ .new_first = true } == 1U);
static_assert(update_params{ .new_second = true } == 2U);
static_assert(update_params{ .new_first = true, .new_second = true } == 3U);
static_assert(update_params{ .old_first = true } == 4U);
static_assert(update_params{ .old_second = true } == 8U);

static constexpr auto update_impl{ tfc::motor::detail::encoder<>::update_impl };
static_assert(update_impl({}) == 0);
static_assert(update_impl({ .new_first = true, .new_second = true, .old_first = true, .old_second = true }) == 0);
static_assert(update_impl({ .new_second = true, .old_second = true }) == 0);
static_assert(update_impl({ .new_first = true, .old_first = true }) == 0);
static_assert(update_impl({ .new_second = true }) == 1);
static_assert(update_impl({ .old_first = true }) == 1);
static_assert(update_impl({ .new_first = true, .old_first = true, .old_second = true }) == 1);
static_assert(update_impl({ .new_first = true, .new_second = true, .old_second = true }) == 1);
static_assert(update_impl({ .new_first = true }) == -1);
static_assert(update_impl({ .old_second = true }) == -1);
static_assert(update_impl({ .new_second = true, .old_first = true, .old_second = true }) == -1);
static_assert(update_impl({ .new_first = true, .new_second = true, .old_first = true }) == -1);
static_assert(update_impl({ .new_first = true, .new_second = true }) == 2);
static_assert(update_impl({ .old_first = true, .old_second = true }) == 2);
static_assert(update_impl({ .new_second = true, .old_first = true }) == -2);
static_assert(update_impl({ .new_first = true, .old_second = true }) == -2);

}  // namespace compile_tests

using namespace mp_units::si::unit_symbols;
namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::expect;
using ut::operator""_test;
using ut::operator|;
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
  using tachometer = tfc::motor::detail::tachometer<mock_bool_slot_t, tfc::testing::clock, buffer_len>;

  "tachometer with single sensor updates internal state"_test = [] {
    test_instance inst{};
    tachometer tacho{ inst.ctx, inst.client, "name", [](std::int64_t) {} };
    for (std::int64_t idx{ 0 }; idx < static_cast<std::int64_t>(buffer_len * 3); idx++) {
      expect(tacho.position_ == idx);
      tacho.update(true);
      tacho.update(false);
    }
  };

  "tachometer with single sensor calls owner"_test = [] {
    test_instance inst{};
    std::int64_t idx{ 1 };
    tachometer tacho{ inst.ctx, inst.client, "name", [&idx](std::int64_t new_value) {
                       expect(idx == new_value) << fmt::format("got {} expected {}", new_value, idx);
                     } };
    for (; idx < static_cast<std::int64_t>(buffer_len * 3); idx++) {
      tacho.update(true);
      tacho.update(false);
    }
  };

  "tachometer with single sensor stores new values to buffer"_test = [] {
    test_instance inst{};
    tachometer tacho{ inst.ctx, inst.client, "name", [](std::int64_t) {} };
    bool new_state{ true };
    for (std::int64_t idx{ 0 }; idx < static_cast<std::int64_t>(buffer_len * 3); idx++) {
      tfc::testing::clock::time_point const later{ tfc::testing::clock::now() + std::chrono::milliseconds{ 1 } };
      tfc::testing::clock::set_ticks(later);
      tacho.update(new_state);
      expect(tacho.buffer_.front().tacho_state == new_state);
      expect(tacho.buffer_.front().time_point == later);
      new_state = !new_state;
    }
  };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"encoder"> enc_test = [] {
PRAGMA_CLANG_WARNING_POP
  // clang-format on
  using encoder_t = tfc::motor::detail::encoder<mock_bool_slot_t, tfc::testing::clock, buffer_len>;
  struct encoder_test {
    test_instance inst{};
    std::function<void(std::int64_t)> cb{ [](std::int64_t) {
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
    encoder_test test{ .cb = [&called](std::int64_t pos) {
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
    encoder_test test{ .cb = [&called](std::int64_t pos) {
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
  "encoder: 0"_test = [] {
    encoder_test test{};

    test.encoder.first_tacho_update(true);
    expect(test.encoder.position_ == -1);

    test.encoder.first_tacho_update(true);
    expect(test.encoder.position_ == -1);
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

  ///	1   0   |   1   0   ->   no movement
  "encoder: 0"_test = [] {
    encoder_test test{};

    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == 1);

    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == 1);
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

  ///   1   1   |   1   1   ->   no movement
  "encoder: 0"_test = [] {
    encoder_test test{};

    test.encoder.first_tacho_update(true);
    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == -2) << test.encoder.position_;

    test.encoder.first_tacho_update(true);
    test.encoder.second_tacho_update(true);
    expect(test.encoder.position_ == -2) << test.encoder.position_;
  };
};

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wglobal-constructors)
[[maybe_unused]] static ut::suite<"notifications"> notify_tests = [] {
PRAGMA_CLANG_WARNING_POP
  // clang-format on
  struct notification_test {
    test_instance inst{};
    tfc::motor::positioner<> positioner{ inst.ctx, inst.client, "name" };
  };
  using mp_units::si::unit_symbols::mm;
  using std::chrono_literals::operator""ms;

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
    test.positioner.notify_at(80 * mm, [](std::error_code) { expect(false); });
    test.positioner.notify_at(101 * mm, [](std::error_code) { expect(false); });
    test.positioner.notify_at(102 * mm, [](std::error_code) { expect(false); });
    bool called{};
    test.positioner.notify_at(90 * mm, [&called](std::error_code err) {
      expect(!err) << err.message();
      called = true;
    });
    test.positioner.increment_position(-11 * mm);
    test.inst.ctx.run_for(1ms);
    expect(called);
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
};
#endif

int main(int argc, char** argv) {
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

  return EXIT_SUCCESS;
}
