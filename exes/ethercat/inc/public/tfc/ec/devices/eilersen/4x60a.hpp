#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <new>
#include <span>
#include <variant>

#include <mp-units/systems/si/si.h>
#include <boost/asio/io_context.hpp>

#include <tfc/confman/observable.hpp>
#include <tfc/confman/read_only.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::eilersen::e4x60a {
enum struct cell_mode_e : std::uint8_t {
  not_used = 0,
  single,
  reference,
  group_1,
  group_2,
  group_1_non_symmetrical,
  group_2_non_symmetrical
};
}  // namespace tfc::ec::devices::eilersen::e4x60a

// used to generate the enum meta conversions for use in members of config
template <>
struct glz::meta<tfc::ec::devices::eilersen::e4x60a::cell_mode_e> {
  static constexpr std::string_view name{ "e4x60a::cell_mode" };
  using enum tfc::ec::devices::eilersen::e4x60a::cell_mode_e;
  // clang-format off
  static constexpr auto value{ glz::enumerate(
    "Not used", not_used,
    "Single", single,
    "Regerence", reference,
    "Group 1", group_1,
    "Group 2", group_2,
    "Group 1 non symmetrical", group_1_non_symmetrical,
    "Group 2 non symmetrical", group_2_non_symmetrical
  ) };
  // clang-format on
};

namespace tfc::ec::devices::eilersen::e4x60a {

namespace asio = boost::asio;
static constexpr std::size_t max_cells{ 4 };
using signal_t = std::int32_t;
using mass_t = mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, std::int64_t>;

struct used_cells {
  std::array<bool, max_cells> cells{};
};

struct used_cell {
  std::uint8_t cell_nr{};
};

template <cell_mode_e mode>
struct get_used_cell {
  static consteval auto impl() {
    if constexpr (mode == cell_mode_e::single) {
      return used_cell{};
    } else {
      return used_cells{};
    }
  }
  using type = decltype(impl());
};
template <cell_mode_e mode>
using get_used_cell_t = typename get_used_cell<mode>::type;
static_assert(std::same_as<get_used_cell_t<cell_mode_e::single>, used_cell>);
static_assert(std::same_as<get_used_cell_t<cell_mode_e::group_1>, used_cells>);

struct calibration_zero {
  std::int64_t read{}; // read_only
  confman::observable<bool> do_calibrate{ false };
};

struct calibration_weight {
  std::int64_t read{}; // read_only
  mass_t weight{};
  confman::observable<bool> do_calibrate{ false };
};

template <cell_mode_e mode>
struct calibration {
  static constexpr auto name{ glz::enum_name_v<mode> };
  calibration_zero calibration_zero{};
  calibration_weight calibration_weight{};
  std::optional<mass_t> tare_weight{};
  mass_t minimum_load{ 1 * mp_units::si::kilogram };
  mass_t maximum_load{ 100 * mp_units::si::kilogram };
  mass_t max_zero_drift{ 500 * mp_units::si::gram };
  bool track_zero_drift{ false };
  // https://adamequipment.co.uk/content/post/d-vs-e-values/?fbclid=IwAR3x2eF7m1RXF9VSk-uxb391Sh_JnK0tK53hBycqdAj5hNdl0KmmZoXgyvk
  mass_t verification_scale_interval{ 10 * mp_units::si::gram };  // so called `e`
  get_used_cell_t<mode> this_cells{};
  std::string group_name{};  // this calibration group name
};

template <>
struct calibration<cell_mode_e::reference> : std::false_type {
  // todo implement
};

template <>
struct calibration<cell_mode_e::group_1_non_symmetrical> : std::false_type {
  // todo implement
};

template <>
struct calibration<cell_mode_e::group_2_non_symmetrical> : std::false_type {
  // todo implement
};

struct not_used : std::monostate {};

using calibration_types = std::variant<not_used,
                                       calibration<cell_mode_e::single>,
                                       calibration<cell_mode_e::group_1>,
                                       calibration<cell_mode_e::group_2>>;

template <typename child_t>
struct calibration_interface {
  auto get_zero() const noexcept -> std::optional<signal_t> {
    return std::visit(
        []<typename visitor_t>(visitor_t&& visitor) -> std::optional<signal_t> {
          if constexpr (std::convertible_to<visitor_t, not_used>) {
            return std::nullopt;
          } else {
            return visitor.calibration_zero.read;
          }
        },
        static_cast<child_t const*>(this)->get_calibration());
  }
  auto get_calibration_weight() const noexcept -> std::optional<std::pair<signal_t, mass_t>> {
    return std::visit(
        []<typename visitor_t>(visitor_t&& visitor) -> std::optional<std::pair<signal_t, mass_t>> {
          if constexpr (std::convertible_to<visitor_t, not_used>) {
            return std::nullopt;
          } else {
            return std::make_pair(visitor.calibration_weight.read, visitor.calibration_weight.weight);
          }
        },
        static_cast<child_t const*>(this)->get_calibration());
  }
  auto get_cells() const noexcept -> std::array<bool, max_cells> {
    std::array<bool, max_cells> result{};
    std::visit(
        [&result]<typename visitor_t>(visitor_t&& visitor) {
          if constexpr (std::convertible_to<visitor_t, not_used>) {
            return;
          } else if constexpr (std::same_as<used_cells, std::remove_cvref_t<decltype(visitor.this_cells)>>) {
            result = visitor.this_cells.cells;
          } else if constexpr (std::same_as<used_cell, std::remove_cvref_t<decltype(visitor.this_cells)>>) {
            if (visitor.this_cells.cell_nr > 0 && visitor.this_cells.cell_nr <= max_cells) {
              result.at(visitor.this_cells.cell_nr - 1) = true;
            }
          } else {
            []<bool flag = false> {
              static_assert(flag, "Unsupported type");
            }
            ();
          }
        },
        static_cast<child_t const*>(this)->get_calibration());
    return result;
  }
  auto get_group_name() const noexcept -> std::optional<std::string_view> {
    return std::visit(
        []<typename visitor_t>(visitor_t&& visitor) -> std::optional<std::string_view> {
          if constexpr (std::convertible_to<visitor_t, not_used>) {
            return std::nullopt;
          } else {
            return visitor.group_name;
          }
        },
        static_cast<child_t const*>(this)->get_calibration());
  }
};

struct calibration_config : calibration_interface<calibration_config> {
  calibration_types calibration_v{ not_used{} };
  confman::observable<bool> sealed{ false };
  auto get_calibration() const noexcept -> calibration_types const& { return calibration_v; }
};

struct calibration_sealed_config : calibration_interface<calibration_sealed_config> {
  calibration_types calibration_v{ not_used{} }; // read_only
  static constexpr bool sealed{ true };
  auto get_calibration() const noexcept -> calibration_types const& { return calibration_v; }
};

using calibration_config_t = std::variant<calibration_config, calibration_sealed_config>;

using variations_t = std::array<calibration_config_t, max_cells>;

struct config {
  variations_t variations{ calibration_config{}, calibration_config{}, calibration_config{}, calibration_config{} };
  // any related config parameters that apply to the whole device can be added here
};
}  // namespace tfc::ec::devices::eilersen::e4x60a

namespace glz {
using glz::operator""_c;
namespace e4x60a = tfc::ec::devices::eilersen::e4x60a;

template <>
struct meta<e4x60a::used_cell> {
  using type = e4x60a::used_cell;
  static constexpr std::string_view name{ "4x60a::used_cell" };
  static constexpr auto value{ glz::object(
      "cell_nr",
      &type::cell_nr,
      tfc::json::schema{ .description = "Pick the cell to be used, 1-4", .minimum = 1LU, .maximum = e4x60a::max_cells }) };
};

template <>
struct meta<e4x60a::used_cells> {
  using type = e4x60a::used_cells;
  static constexpr std::string_view name{ "4x60a::used_cells" };
  // clang-format off
  static constexpr auto value{ glz::object(
    "cells", &type::cells, "Cells associated to this group"
    ) };
  // clang-format on
};

template <>
struct meta<e4x60a::calibration_zero> {
  using type = e4x60a::calibration_zero;
  static constexpr std::string_view name{ "4x60a::calibration_zero" };
  // clang-format off
  static constexpr auto value{ glz::object(
    "signal_read", &type::read, tfc::json::schema{ .description = "The read value of the calibration zero", .read_only = true },
    "do_calibrate_zero", &type::do_calibrate, "Set to true to calibrate the zero"
  ) };
  // clang-format on
};

template <>
struct meta<e4x60a::calibration_weight> {
  using type = e4x60a::calibration_weight;
  static constexpr std::string_view name{ "4x60a::calibration_weight" };
  // clang-format off
  static constexpr auto value{ glz::object(
    "signal_read", &type::read, tfc::json::schema{ .description="The actual value signal from load cell/cells of the calibration weight", .read_only = true },
    "weight", &type::weight, tfc::json::schema{ .description = "Calibration weight", .default_value = 1UL },
    "do_calibrate_with_load", &type::do_calibrate, "Set to true to calibrate the weight"
  ) };
  // clang-format on
};

template <e4x60a::cell_mode_e mode>
struct meta<e4x60a::calibration<mode>> {
  using type = e4x60a::calibration<mode>;
  // static constexpr std::string_view prefix{ "eilersen::4x60a::calibration::" };
  static constexpr std::string_view name{ type::name };
  // clang-format off
  static constexpr auto value{ glz::object(
      type::name, &type::name, tfc::json::schema{ .description="The mode of this calibration.", .constant = type::name },
      "calibration_zero", &type::calibration_zero, "Calibrate zero factors.",
      "calibration_weight", &type::calibration_weight, "Calibrate weight factors.",
      "tare_weight", &type::tare_weight, "The tare weight, will be subtracted from the weight. This is the weight of the container such as a box or a bag.",
      "minimum_load", &type::minimum_load, "The minimum load weight, won't report values below this.",
      "maximum_load", &type::maximum_load, "The maximum load weight, won't report values above this and load cells could get damaged.",
      "max_zero_drift", &type::max_zero_drift, "The maximum zero drift, if zero has drifted greater than this value the weigher will be in errorous state.",
      "track_zero_drift", &type::track_zero_drift, "If true the zero drift will be tracked when stable for 1 sec zero reading. Will update zero but not change the config.",
      "scale_resolution", &type::verification_scale_interval, "The smallest scale increment that can be used to determine price by weight in commercial transactions known as `e`.",
      "cell/cells", &type::this_cells, "The cells to use for this calibration.",
      "group_name", &type::group_name, tfc::json::schema{
        .description= "The name of this calibration group, used for naming IPC signal. Requires restart of the process.",
        .min_length = 3, .max_length = 30, .pattern = "^[a-zA-Z0-9_]+$" }
      ) };
  // clang-format on
};

template <>
struct meta<e4x60a::not_used> {
  static constexpr std::string_view name{ "Not used" };
};

template <>
struct meta<e4x60a::calibration_config> {
  using type = e4x60a::calibration_config;
  static constexpr std::string_view name{ "Calibration Config" };
  // distingusing keys for the two variants, required for glaze
  static constexpr auto value{ glz::object("calibration",
                                           &type::calibration_v,
                                           "make_seal",
                                           &type::sealed) };
};

template <>
struct meta<e4x60a::calibration_sealed_config> {
  using type = e4x60a::calibration_sealed_config;
  static constexpr std::string_view name{ "Sealed Calibration Config" };
  static constexpr auto value{ glz::object("calibration",
                                           &type::calibration_v,
                                           tfc::json::schema{ .read_only = true },
                                           "sealed",
                                           &type::sealed,
                                           tfc::json::schema{ .constant = true }) };
};

template <>
struct meta<e4x60a::config> {
  using type = e4x60a::config;
  static constexpr std::string_view name{ "e4x60a::config" };
  static constexpr auto value{ glz::object("variations", &type::variations  // todo json schema set as hidden
    ) };
};
}  // namespace glz

namespace tfc::ec::devices::eilersen::e4x60a {

struct lc_status {
  bool cell_1_broken : 1 {};
  bool cell_2_broken : 1 {};
  bool cell_3_broken : 1 {};
  bool cell_4_broken : 1 {};
  std::uint8_t unused : 4 {};
  bool broken(std::size_t idx) const noexcept {
    switch (idx) {
      case 0: return cell_1_broken;
      case 1: return cell_2_broken;
      case 2: return cell_3_broken;
      case 3: return cell_4_broken;
      default: return true; // invalid index
    }
  }
};

#pragma pack(push, 1)
struct pdo_input {
  lc_status status{};
  std::uint8_t nr_of_inputs{};
  signal_t weight_signals[max_cells]{};
};
#pragma pack(pop)
static_assert(sizeof(pdo_input) == 18);

#pragma pack(push, 1)
struct pdo_output {
  // unused
  std::uint8_t control{};
  std::uint16_t measurement_time{};
  std::uint8_t filter_number{};
};
#pragma pack(pop)
static_assert(sizeof(pdo_output) == 4);

template <typename manager_client_type,
          template <typename description_t, typename manager_client_t> typename ipc_signal_t = ipc::signal>
class e4x60a final : public base {
public:
  e4x60a(asio::io_context& ctx, manager_client_type& client, std::uint16_t slave_index)
      : base{ slave_index }, ctx_{ ctx }, client_{ client } {
    std::size_t idx{ 0 }; // todo support multiple
    if (auto* itm{ std::get_if<calibration_config>(&config_->variations.at(idx)) }) {
      itm->sealed.observe(std::bind_front(&e4x60a::make_seal, this, idx));
      std::visit([this, idx]<typename cal_t>(cal_t& cal){
        if constexpr (!std::convertible_to<not_used, cal_t>) {
          cal.calibration_zero.do_calibrate.observe(std::bind_front(&e4x60a::calibrate_zero, this, idx));
          cal.calibration_weight.do_calibrate.observe(std::bind_front(&e4x60a::calibrate_weight, this, idx));
        }
      }, itm->calibration_v);
    }
  }

  void do_calibration(auto& calibration) {
    calibration.read = static_cast<signal_t>(last_cumilated_signal_);
    calibration.do_calibrate = false;
  }

  void calibrate_zero(std::size_t idx, [[maybe_unused]] bool new_value, [[maybe_unused]] bool old_value) noexcept {
    if (new_value) {
      if (std::holds_alternative<calibration_config>(config_->variations.at(idx))) {
        auto change{ config_.make_change() };
        auto& itm{ std::get<calibration_config>(change->variations.at(idx)) };
        std::visit([this]<typename cal_t>(cal_t& cal) {
          if constexpr (!std::convertible_to<not_used, cal_t>) {
            this->do_calibration(cal.calibration_zero);
          }
        }, itm.calibration_v);
      }
    }
  }

  void calibrate_weight(std::size_t idx, [[maybe_unused]] bool new_value, [[maybe_unused]] bool old_value) noexcept {
    if (new_value) {
      if (std::holds_alternative<calibration_config>(config_->variations.at(idx))) {
        auto change{ config_.make_change() };
        auto& itm{ std::get<calibration_config>(change->variations.at(idx)) };
        std::visit([this]<typename cal_t>(cal_t& cal) {
          if constexpr (!std::convertible_to<not_used, cal_t>) {
            this->do_calibration(cal.calibration_weight);
          }
        }, itm.calibration_v);
      }
    }
  }

  void make_seal(std::size_t idx, [[maybe_unused]] bool new_value, [[maybe_unused]] bool old_value) noexcept {
    if (new_value) {
      if (std::holds_alternative<calibration_config>(config_->variations.at(idx))) {
        auto change{ config_.make_change() };
        auto& itm{ std::get<calibration_config>(change->variations.at(idx)) };
        auto calibration_params{ std::move(itm.calibration_v) };
        change->variations.at(idx) = calibration_sealed_config{ .calibration_v = std::move(calibration_params) };
      }
    }
  }

  void process_data(std::span<std::byte> in, std::span<std::byte> out) final {
    if (in.size() != sizeof(pdo_input)) {
      this->logger_.warn("Invalid input data size, expected {}, got {}", sizeof(pdo_input), in.size());
      return;
    }
    [[maybe_unused]] auto* input = std::launder(reinterpret_cast<pdo_input*>(in.data()));

    // todo support multiple variations
    auto& group_1{ config_->variations.at(0) };
    auto value{ std::visit(
        [this, input](auto& group_1_cal) -> ipc::details::mass_t {
          last_cumilated_signal_ = 0;
          std::size_t idx{};
          for (auto using_cell : group_1_cal.get_cells()) {
            if (using_cell) {
              if (input->status.broken(idx)) {
                return std::unexpected{ ipc::details::mass_error_e::cell_fault };
              }
// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wunsafe-buffer-usage)
              last_cumilated_signal_ += input->weight_signals[idx];
PRAGMA_CLANG_WARNING_POP
// clang-format on
            }
            idx++;
          }

          if (auto zero{ group_1_cal.get_zero() }) {
            last_cumilated_signal_ -= *zero;
          }
          if (auto calibration{ group_1_cal.get_calibration_weight() }) {
            auto [calibration_signal, calibration_mass] = calibration.value();
            if (calibration_signal == 0) {
              return std::unexpected{ ipc::details::mass_error_e::not_calibrated };
            }
            mass_t actual_mass{ ((last_cumilated_signal_ * calibration_mass.numerical_value_) / calibration_signal) * mass_t::reference };
            return actual_mass;
          }
          return std::unexpected{ ipc::details::mass_error_e::not_calibrated };
        },
        group_1) };
    mass_.async_send(value, [this](std::error_code const& err, auto) {
      if (err) {
        logger_.warn("Unable to send mass signal: {}", err.message());
      }
    });
  }

  static constexpr uint32_t product_code = 0x1040;
  static constexpr uint32_t vendor_id = 0x726;
  asio::io_context& ctx_;
  ipc_ruler::ipc_manager_client& client_;
  confman::config<config> config_{ ctx_, fmt::format("eilersen_weighing_module_{}", slave_index_) };
  // todo make section struct to cover the config, for now only one output is generated
  std::int64_t last_cumilated_signal_{};
  ipc_signal_t<ipc::details::type_mass, ipc_ruler::ipc_manager_client&> mass_{ ctx_, client_, "group_1",
                                                                           "Weigher output for group 1" };
};
}  // namespace tfc::ec::devices::eilersen::e4x60a