#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <new>
#include <ranges>
#include <span>
#include <utility>

#include <mp-units/bits/quantity_concepts.h>
#include <fmt/format.h>

#include <tfc/ec/soem_interface.hpp>
#include <tfc/logger.hpp>

namespace tfc::ec::devices {
template <typename setting_t>
concept setting_c = requires {
  std::remove_cvref_t<setting_t>::index;
  std::remove_cvref_t<setting_t>::value;
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

class base {
public:
  virtual ~base();

  // Default behaviour no data processing
  virtual void process_data(std::span<std::byte>, std::span<std::byte>) = 0;

  // Default behaviour, no setup
  virtual auto setup() -> int { return 1; }

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
    return base::sdo_write(in.index, in.value.numerical_value_);
  }

protected:
  explicit base(uint16_t slave_index) : slave_index_(slave_index), logger_(fmt::format("Ethercat slave {}", slave_index)) {}

  const uint16_t slave_index_{};
  tfc::logger::logger logger_;

private:
  std::function<
      ecx::working_counter_t(ecx::index_t, ecx::complete_access_t, std::span<std::byte>, std::chrono::microseconds)>
      sdo_write_{};
};

class default_device final : public base {
public:
  ~default_device() final;

  explicit default_device(uint16_t const slave_index) : base(slave_index) {}

  void process_data(std::span<std::byte>, std::span<std::byte>) noexcept final {}

  auto setup() -> int final { return TRUE; }
};
}  // namespace tfc::ec::devices
