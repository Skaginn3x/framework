#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <span>

#include <mp-units/systems/si/si.h>
// todo pounds
// #include <mp-units/systems/international/international.h>
#include <boost/asio/io_context.hpp>

#include <tfc/utils/units_glaze_meta.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/confman/read_only.hpp>

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
} // namespace tfc::ec::devices::eilersen::e4x60a

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
using weight_t = mp_units::quantity<mp_units::si::milli<mp_units::si::gram>>;

struct used_cells {
  bool cell_1{};
  bool cell_2{};
  bool cell_3{};
  bool cell_4{};
};

struct used_cell {
  std::uint8_t cell_nr{};
};

template <cell_mode_e mode>
struct get_used_cell {
  static consteval auto impl() {
    if constexpr (mode == cell_mode_e::single) {
      return used_cell{};
    }
    else {
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
  confman::read_only<signal_t> read{};
  confman::observable<bool> do_calibrate{ false };
};

struct calibration_weight {
  confman::read_only<signal_t> read{};
  weight_t weight{};
  confman::observable<bool> do_calibrate{ false };
};

// todo support pounds ?
template <cell_mode_e mode>
struct calibration {
  static constexpr auto name{ glz::enum_name_v<mode> };
  calibration_zero calibration_zero{};
  calibration_weight calibration_weight{};
  std::optional<weight_t> tare_weight{};
  weight_t minimum_load{ 1 * mp_units::si::kilogram };
  weight_t maximum_load{ 100 * mp_units::si::kilogram };
  weight_t max_zero_drift{ 500 * mp_units::si::gram };
  bool track_zero_drift{ false };
  // https://adamequipment.co.uk/content/post/d-vs-e-values/?fbclid=IwAR3x2eF7m1RXF9VSk-uxb391Sh_JnK0tK53hBycqdAj5hNdl0KmmZoXgyvk
  weight_t verification_scale_interval{ 10 * mp_units::si::gram }; // so called `e`
  get_used_cell_t<mode> this_cells{};
  // Below we declare boolean actions to perform on the device throught the config API
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

struct not_used : std::monostate {
};

using calibration_types =
std::variant<not_used, calibration<cell_mode_e::single>, calibration<cell_mode_e::group_1>, calibration<
               cell_mode_e::group_2>>;

struct calibration_config {
  calibration_types calibration_v{ not_used{} };
  bool sealed{ false };
};

struct calibration_sealed_config {
  confman::read_only<calibration_types> calibration_v{ not_used{} };
  static constexpr bool sealed{ true };
};

using calibration_config_t = std::variant<calibration_config, calibration_sealed_config>;

using variations_t = std::array<calibration_config_t, max_cells>;

struct config {
  variations_t variations{ calibration_config{}, calibration_config{}, calibration_config{}, calibration_config{} };
  // any related config parameters that apply to the whole device can be added here
};
} // namespace tfc::ec::devices::eilersen::e4x60a

namespace glz {
using glz::operator""_c;
namespace e4x60a = tfc::ec::devices::eilersen::e4x60a;

template <>
struct meta<e4x60a::used_cell> {
  using type = e4x60a::used_cell;
  static constexpr std::string_view name{ "4x60a::used_cell" };
  static constexpr auto value{ glz::object("cell_nr", &type::cell_nr, tfc::json::schema{ .description = "Pick the cell to be used, 1-4", .minimum = 1LU, .maximum = e4x60a::max_cells }) };
};

template <>
struct meta<e4x60a::used_cells> {
  using type = e4x60a::used_cells;
  static constexpr std::string_view name{ "4x60a::used_cells" };
  // clang-format off
  static constexpr auto value{ glz::object(
    "cell_1", &type::cell_1, "Use cell 1 in this group",
    "cell_2", &type::cell_2, "Use cell 2 in this group",
    "cell_3", &type::cell_3, "Use cell 3 in this group",
    "cell_4", &type::cell_4, "Use cell 4 in this group"
    ) };
  // clang-format on
};

template <>
struct meta<e4x60a::calibration_zero> {
  using type = e4x60a::calibration_zero;
  static constexpr std::string_view name{ "4x60a::calibration_zero" };
  // clang-format off
  static constexpr auto value{ glz::object(
    "signal_read", &type::read, "The read value of the calibration zero",
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
    "signal_read", &type::read, "The actual value signal from load cell/cells of the calibration weight",
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
      "type", &type::name, tfc::json::schema{ .description="The mode of this calibration.", .constant = type::name },
      "calibration_zero", &type::calibration_zero, "Calibrate zero factors.",
      "calibration_weight", &type::calibration_weight, "Calibrate weight factors.",
      "tare_weight", &type::tare_weight, "The tare weight, will be subtracted from the weight. This is the weight of the container such as a box or a bag.",
      "minimum_load", &type::minimum_load, "The minimum load weight, won't report values below this.",
      "maximum_load", &type::maximum_load, "The maximum load weight, won't report values above this and load cells could get damaged.",
      "max_zero_drift", &type::max_zero_drift, "The maximum zero drift, if zero has drifted greater than this value the weigher will be in errorous state.",
      "track_zero_drift", &type::track_zero_drift, "If true the zero drift will be tracked when stable for 1 sec zero reading. Will update zero but not change the config.",
      "verification_scale_interval", &type::verification_scale_interval, "The smallest scale increment that can be used to determine price by weight in commercial transactions known as `e`.",
      "cell/cells", &type::this_cells, "The cells to use for this calibration."
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
  static constexpr auto value{ glz::object("calibration", &type::calibration_v, "sealed", &type::sealed) };
};

template <>
struct meta<e4x60a::calibration_sealed_config> {
  using type = e4x60a::calibration_sealed_config;
  static constexpr std::string_view name{ "Sealed Calibration Config" };
  static constexpr auto value{ glz::object("calibration", &type::calibration_v, "sealed", &type::sealed) };
};

template <>
struct meta<e4x60a::config> {
  using type = e4x60a::config;
  static constexpr std::string_view name{ "e4x60a::config" };
  static constexpr auto value{ glz::object("variations", &type::variations) };
};
} // namespace glz


namespace tfc::ec::devices::eilersen::e4x60a {

struct lc_status {
  bool cell_1_broken : 1 {};
  bool cell_2_broken : 1 {};
  bool cell_3_broken : 1 {};
  bool cell_4_broken : 1 {};
  std::uint8_t unused : 4 {};
};

#pragma pack(push, 1)
struct pdo_input {
  lc_status status{};
  std::uint8_t nr_of_inputs{};
  signal_t weight_1{};
  signal_t weight_2{};
  signal_t weight_3{};
  signal_t weight_4{};
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
          template <typename description_t, typename manager_client_t> typename signal_t = ipc::signal>
class e4x60a final : public base {
public:
  e4x60a(asio::io_context& ctx, manager_client_type& client, std::uint16_t slave_index)
    : base{ slave_index }, ctx_{ ctx } {
  }

  void process_data(std::span<std::byte> in, std::span<std::byte> out) final {
    if (in.size() != sizeof(pdo_input)) {
      this->logger_.warn("Invalid input data size, expected {}, got {}", sizeof(pdo_input), in.size());
      return;
    }
    [[maybe_unused]] auto* input = std::launder(reinterpret_cast<pdo_input*>(in.data()));
    // if (input->nr_of_cells_at_power_on > max_cells) {
    //   this->logger_.warn("Invalid number of active cells, expected less than {}, got {}", max_cells,
    //                      input->nr_of_cells_at_power_on);
    //   return;
    // }
    // for (auto used_cell_nr{ 0U }; used_cell_nr < input->nr_of_cells_at_power_on; ++used_cell_nr) {
    // }
  }

  static constexpr uint32_t product_code = 0x1040;
  static constexpr uint32_t vendor_id = 0x726;
  asio::io_context& ctx_;
  confman::config<config> config_{
    ctx_, "weigher"
  };
};
} // namespace tfc::ec::devices::eilersen::e4x60a
