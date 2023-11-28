#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <span>

#include <mp-units/systems/si/si.h>
#include <boost/asio/io_context.hpp>

#include <tfc/confman/observable.hpp>
#include <tfc/confman/read_only.hpp>

#include "tfc/ec/devices/base.hpp"

namespace tfc::ec::devices::eilersen::e4x60a {

namespace asio = boost::asio;

struct lc_status {
  bool cell_1_working : 1 {};
  bool cell_2_working : 1 {};
  bool cell_3_working : 1 {};
  bool cell_4_working : 1 {};
  std::uint8_t unused : 4 {};
};

using weigth_t = std::uint32_t;

#pragma pack(push, 1)
struct pdo_input {
  lc_status status{};
  std::uint8_t nr_of_cells_at_power_on{};
  weigth_t weight_1{};
  weigth_t weight_2{};
  weigth_t weight_3{};
  weigth_t weight_4{};
};
#pragma pack(pop)
static_assert(sizeof(pdo_input) == 18);

#pragma pack(push, 1)
struct pdo_output {  // unused
  std::uint8_t control{};
  std::uint16_t measurement_time{};
  std::uint8_t filter_number{};
};
#pragma pack(pop)
static_assert(sizeof(pdo_output) == 4);

enum struct mode_e : std::uint8_t {
  not_used = 0,
  single,
  reference,
  group_1,
  group_2,
  group_1_non_symmetrical,
  group_2_non_symmetrical
};
template <mode_e mode>
struct calibration {
  static constexpr auto type{ glz::enum_name_v<mode> };
  confman::read_only<weigth_t> zero{};
  using milligram_64bit = mp_units::quantity<mp_units::si::milli<mp_units::si::gram>, uint64_t>;
  milligram_64bit calibration_load{};
  confman::read_only<weigth_t> calibration_load_read{};
  struct glaze {
    static constexpr std::string_view prefix{ "eilersen::4x60a::calibration::" };
    static constexpr auto name{ tfc::stx::string_view_join_v<prefix, glz::enum_name_v<mode>> };
    static constexpr auto value{ glz::object(
        "type",
        &calibration::type,
        "The mode of this calibration."
        "zero",
        &calibration::zero,
        "Read of load cell actual value when no load other then mechanics is applied.",
        "calibration_load",
        &calibration::calibration_load,
        "The calibration load weight",
        "calibration_load_read",
        &calibration::calibration_load_read,
        "Read of load cell actual value when calibration load is applied onto mechanics.") };
  };
};
template <>
struct calibration<mode_e::reference> : std::false_type {
  // todo implement
};
template <>
struct calibration<mode_e::group_1_non_symmetrical> : std::false_type {
  // todo implement
};
template <>
struct calibration<mode_e::group_2_non_symmetrical> : std::false_type {
  // todo implement
};
struct not_used : std::monostate {};

using mode_t =
    std::variant<not_used, calibration<mode_e::single>, calibration<mode_e::group_1>, calibration<mode_e::group_2>>;
using variations_t = std::array<mode_t, 4>;

template <typename manager_client_type,
          template <typename description_t, typename manager_client_t> typename signal_t = ipc::signal>
class e4x60a final : public base {
public:
  e4x60a(asio::io_context& ctx, manager_client_type& client, std::uint16_t slave_index) : base{ slave_index }, ctx_{ ctx } {}
  struct config {
    variations_t variations{ not_used{}, not_used{}, not_used{}, not_used{} };
    struct glaze {
      static constexpr std::string_view name{ "eilersen::4x60a" };
      static constexpr auto value{ glz::object("cells", &config::variations) };
    };
  };
  void process_data(std::span<std::byte> in, std::span<std::byte> out) final {
    if (in.size() != sizeof(pdo_input)) {
      this->logger_.warn("Invalid input data size, expected {}, got {}", sizeof(pdo_input), in.size());
      return;
    }
    auto* input = std::launder(reinterpret_cast<pdo_input*>(in.data()));
    if (input->nr_of_cells_at_power_on > sizeof(config::cells)) {
      this->logger_.warn("Invalid number of active cells, expected less than {}, got {}", sizeof(config::cells),
                         input->nr_of_cells_at_power_on);
      return;
    }
    for (auto used_cell_nr{ 0U }; used_cell_nr < input->nr_of_cells_at_power_on; ++used_cell_nr) {
    }
  }
  static constexpr uint32_t product_code = 0x1040;
  static constexpr uint32_t vendor_id = 0x726;
  asio::io_context& ctx_;
  confman::config<config> config_{
    ctx_, "weigher"
  };
};
}  // namespace tfc::ec::devices::eilersen::e4x60a

template <>
struct glz::meta<tfc::ec::devices::eilersen::e4x60a::mode_e> {
  static constexpr std::string_view name{ "mode" };
  using enum tfc::ec::devices::eilersen::e4x60a::mode_e;
  static constexpr auto value{
    glz::enumerate("not_used", not_used, "single", single, "group_1", group_1, "group_2", group_2)
  };
};

template <>
struct glz::meta<tfc::ec::devices::eilersen::e4x60a::not_used> {
  static constexpr std::string_view name{ "Not used" };
};
