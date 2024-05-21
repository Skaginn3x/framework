#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <new>
#include <ranges>
#include <span>
#include <utility>

#include <fmt/format.h>
#include <mp-units/bits/quantity_concepts.h>

#include <tfc/ec/soem_interface.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/concepts.hpp>

namespace tfc::ec::devices {
template <typename setting_t>
concept setting_c = requires {
  std::remove_cvref_t<setting_t>::index;
  std::remove_cvref_t<setting_t>::value;
};
template <typename setting_t>
concept optional_setting_c = requires {
  requires stx::is_optional<std::remove_cvref_t<setting_t>>;
  requires setting_c<typename std::remove_cvref_t<setting_t>::value_type>;
};
template <typename setting_t>
concept trivial_setting_c = requires {
  requires std::is_integral_v<decltype(std::remove_cvref_t<setting_t>::value)> ||
               std::is_enum_v<decltype(std::remove_cvref_t<setting_t>::value)>;
  requires setting_c<setting_t>;
};

template <typename setting_t>
concept chrono_setting_c = requires {
  requires std::is_member_function_pointer_v<decltype(&std::remove_cvref_t<setting_t>::type::count)>;
  requires setting_c<setting_t>;
};

template <typename setting_t>
concept mp_units_quantity_setting_c = requires {
  requires mp_units::Quantity<typename std::remove_cvref_t<setting_t>::type>;
  requires setting_c<setting_t>;
};

namespace details {
// General arg_type for function types
template <std::size_t N, typename>
struct arg_type;

// Specialization for free function types
template <std::size_t N, typename Ret, typename... Args>
struct arg_type<N, Ret(Args...)> {
  static_assert(N < sizeof...(Args), "Argument index out of range.");
  using type = std::tuple_element_t<N, std::tuple<Args...>>;
};

// Specialization for member function types
template <std::size_t N, typename Ret, typename Class, typename... Args>
struct arg_type<N, Ret (Class::*)(Args...)> {
  static_assert(N < sizeof...(Args), "Argument index out of range.");
  using type = std::tuple_element_t<N, std::tuple<Args...>>;
};

// Specialization for noexcept member function types
template <std::size_t N, typename Ret, typename Class, typename... Args>
struct arg_type<N, Ret (Class::*)(Args...) noexcept> {
  static_assert(N < sizeof...(Args), "Argument index out of range.");
  using type = std::tuple_element_t<N, std::tuple<Args...>>;
};

template <typename some_t>
using first_arg_t = typename arg_type<0, some_t>::type;
template <typename some_t>
using second_arg_t = typename arg_type<1, some_t>::type;

template <typename some_t>
concept is_first_arg_const_ref =
    std::is_reference_v<first_arg_t<some_t>> && std::is_const_v<std::remove_reference_t<first_arg_t<some_t>>>;

template <typename some_t>
concept is_second_arg_ref =
    std::is_reference_v<second_arg_t<some_t>> && !std::is_const_v<std::remove_reference_t<second_arg_t<some_t>>>;

template <typename some_t>
using pdo_error_t = decltype(std::declval<some_t>().pdo_error());

template <typename some_t>
using setup_driver_t = decltype(std::declval<some_t>().setup_driver());

struct Foo {
  void foo(int, double) {}
  void bar(int&, double&) {}
  void foobar(int const&, double const&) {}
};
static_assert(std::same_as<first_arg_t<decltype(&Foo::foo)>, int>);
static_assert(std::same_as<first_arg_t<decltype(&Foo::bar)>, int&>);
static_assert(std::same_as<first_arg_t<decltype(&Foo::foobar)>, int const&>);
static_assert(std::same_as<second_arg_t<decltype(&Foo::foo)>, double>);
static_assert(std::same_as<second_arg_t<decltype(&Foo::bar)>, double&>);
static_assert(std::same_as<second_arg_t<decltype(&Foo::foobar)>, double const&>);

static_assert(!is_first_arg_const_ref<decltype(&Foo::foo)>);
static_assert(!is_first_arg_const_ref<decltype(&Foo::bar)>);
static_assert(is_first_arg_const_ref<decltype(&Foo::foobar)>);
static_assert(!is_second_arg_ref<decltype(&Foo::foo)>);
static_assert(is_second_arg_ref<decltype(&Foo::bar)>);
static_assert(!is_second_arg_ref<decltype(&Foo::foobar)>);

}  // namespace details

template <typename impl_t>
class base {
public:
  using default_t = std::span<std::uint8_t>;
  base(base const&) = delete;
  auto operator=(base const&) -> base& = delete;
  base(base&&) = default;
  base& operator=(base&&) = default;

  // Default behaviour no data processing
  void process_data(default_t input, default_t output) {
    static_assert(std::is_member_function_pointer_v<decltype(&impl_t::pdo_cycle)>, "impl_t must have method pdo_cycle");
    using pdo_cycle_func_t = decltype(&impl_t::pdo_cycle);  // is function pointer
    using input_pdo = std::decay_t<details::first_arg_t<pdo_cycle_func_t>>;
    using output_pdo = std::decay_t<details::second_arg_t<pdo_cycle_func_t>>;

    // Verify that the declaration of arguments are correct
    if constexpr (!std::same_as<input_pdo, default_t>) {
      static_assert(stx::is_constexpr_default_constructible_v<input_pdo>);
      static_assert(details::is_first_arg_const_ref<pdo_cycle_func_t>,
                    "impl_t::pdo_cycle must have first argument as const reference");
    }
    if constexpr (!std::same_as<output_pdo, default_t>) {
      static_assert(stx::is_constexpr_default_constructible_v<output_pdo>);
      static_assert(details::is_second_arg_ref<pdo_cycle_func_t>,
                    "impl_t::pdo_cycle must have second argument as reference, const is not allowed");
    }

    auto extract_data{ [this]<typename pdo_buffer_t>(pdo_buffer_t* resulting_buffer, default_t actual_buffer,
                                                     bool& valid_flag) {
      if constexpr (std::same_as<pdo_buffer_t, default_t>) {
        resulting_buffer = &actual_buffer;
      } else {
        if (actual_buffer.size() != sizeof(pdo_buffer_t)) {
          if (valid_flag) {
            logger_.warn("Pdo buffer size mismatch, expected {} bytes, got {} bytes", sizeof(pdo_buffer_t),
                         actual_buffer.size());
            valid_flag = false;
          }
          if constexpr (stx::is_detected_v<details::pdo_error_t, impl_t>) {
            static_cast<impl_t*>(this)->pdo_error();
          }
          return false;
        }
        valid_flag = true;
        // clang-format off
        PRAGMA_CLANG_WARNING_PUSH_OFF(-Wunsafe-buffer-usage)
        // clang-format on
        resulting_buffer = reinterpret_cast<pdo_buffer_t*>(actual_buffer.data());
        PRAGMA_CLANG_WARNING_POP
      }
      return true;
    } };

    input_pdo* input_data{ nullptr };
    if (!extract_data(input_data, input, input_buffer_valid_)) {
      return;
    }
    output_pdo* output_data{ nullptr };
    if (!extract_data(output_data, output, output_buffer_valid_)) {
      return;
    }
    static_cast<impl_t*>(this)->pdo_cycle(*input_data, *output_data);
  }

  // Default behaviour, no setup
  auto setup() -> int {
    // If impl_t has method setup_driver, call it
    if constexpr (stx::is_detected_v<details::setup_driver_t, impl_t>) {
      return static_cast<impl_t*>(this)->setup_driver();
    }
    return 1;
  }

  void set_sdo_write_cb(auto&& cb) { sdo_write_ = std::forward<decltype(cb)>(cb); }

  auto sdo_write(ecx::index_t idx,
                 ecx::complete_access_t acc,
                 std::span<std::byte> const& data,
                 std::chrono::microseconds timeout) const -> ecx::working_counter_t {
    if (sdo_write_) {
      return std::invoke(sdo_write_, idx, acc, data, timeout);
    }
    logger_.warn("Sdo write callback not set");
    return {};
  }

  template <std::integral integral_t>
  auto sdo_write(ecx::index_t idx, integral_t value) const -> ecx::working_counter_t {
    if (sdo_write_) {
      std::span data{ std::launder(reinterpret_cast<std::byte*>(&value)), sizeof(value) };
      return std::invoke(sdo_write_, idx, false, data, ecx::constants::timeout_safe);
    }
    logger_.warn("Sdo write callback not set");
    return {};
  }

  template <trivial_setting_c setting_t>
  auto sdo_write(setting_t&& in) const {
    using value_t = decltype(std::remove_cvref_t<setting_t>::value);
    if constexpr (std::is_enum_v<value_t>) {
      return base::sdo_write(in.index, std::to_underlying(in.value));
    } else if constexpr (std::is_integral_v<value_t>) {
      return base::sdo_write(in.index, in.value);
    } else {
      []<bool flag = false>() {
        static_assert(flag);
      }
      ();
    }
  }

  template <chrono_setting_c setting_t>
  auto sdo_write(setting_t&& in) const {
    return base::sdo_write(in.index, in.value.count());
  }

  template <mp_units_quantity_setting_c setting_t>
  auto sdo_write(setting_t&& in) const {
    return base::sdo_write(in.index, in.value.numerical_value_ref_in(decltype(in.value)::unit));
  }

  template <optional_setting_c setting_t>
  auto sdo_write(setting_t&& in) const {
    if (in.has_value()) {
      return sdo_write(in.value());
    }
    return 0;
  }

protected:
  explicit base(uint16_t slave_index) : slave_index_(slave_index), logger_(fmt::format("Ethercat slave {}", slave_index)) {}

  const uint16_t slave_index_{};
  tfc::logger::logger logger_;

private:
  std::function<
      ecx::working_counter_t(ecx::index_t, ecx::complete_access_t, std::span<std::byte>, std::chrono::microseconds)>
      sdo_write_{};
  bool output_buffer_valid_{ true };
  bool input_buffer_valid_{ true };
};

class default_device final : public base<default_device> {
public:
  explicit default_device(uint16_t const slave_index) : base(slave_index) {
    logger_.warn("No device found for slave {}", slave_index);
  }

  void pdo_cycle(std::span<std::uint8_t>, std::span<std::uint8_t>) noexcept {}
};

using foo = details::first_arg_t<decltype(&default_device::pdo_cycle)>;
static_assert(std::same_as<foo, std::span<std::uint8_t>>);

}  // namespace tfc::ec::devices
