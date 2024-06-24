#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <expected>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <mp-units/systems/angular.h>
#include <mp-units/systems/si.h>
#include <stduuid/uuid.h>
#include <glaze/core/context.hpp>

#include <tfc/stx/basic_fixed_string.hpp>

namespace glz {
struct parse_error;
}

namespace pcg_extras {
template <typename>
class seed_seq_from;
}

namespace tfc::ipc::item {

namespace details {
enum struct category_e : std::uint8_t {
  unknown = 0,
  box,
  tub,
  bag,
  pallet,
  fish,
  meat,
  poultry,
  ice,
  // to be continued
};
enum struct quality_e : std::uint8_t {
  unknown = 0,
  inferior = 10,  // (lowest grade)
  weak = 20,
  ordinary = 30,
  exceptional = 40,
  superior = 50,  // (highest grade)
};
struct color {
  std::uint8_t red{ 0 };
  std::uint8_t green{ 0 };
  std::uint8_t blue{ 0 };
  [[nodiscard]] auto operator==(color const&) const noexcept -> bool = default;
};
struct supplier {
  std::string name{};
  std::string contact_info{};
  std::string country{};  // todo should this be an enum
  [[nodiscard]] auto operator==(supplier const&) const noexcept -> bool = default;
};
struct destination {
  // todo decide what destination can be
  [[nodiscard]] auto operator==(destination const&) const noexcept -> bool = default;
};
}  // namespace details

// Food and Agriculture Organization of the United Nations
// Reference: https://www.fao.org/fishery/en/collection/asfis/en
namespace fao {
struct species {
  bool outside_spec{ false };  // if struct is used for outside the specification, represented with `!` as first character
  using string_type = tfc::stx::basic_fixed_string<char, 3>;
  string_type code{};
  static constexpr auto from_3a(const std::string_view marked_code) -> std::optional<species> {
    species ret_value;
    if (marked_code.length() != 3 && marked_code.length() != 4) {
      return std::nullopt;
    }
    if (marked_code.length() == 4) {
      if (marked_code[0] != '!') {
        return std::nullopt;
      }
      ret_value.outside_spec = true;
      for (unsigned int i = 0; i < 3; i++) {
        ret_value.code[i] = marked_code[i + 1];
      }
      return ret_value;
    }
    ret_value.outside_spec = false;
    for (unsigned int i = 0; i < 3; i++) {
      ret_value.code[i] = marked_code[i];
    }
    return ret_value;
  }
  static constexpr auto from_int(std::uint16_t input) -> std::optional<species> {
    species res{};
    if (input >= offset) {
      res.outside_spec = true;
      input -= offset;
    }
    unsigned int cnt{ 3 };
    while (input > 0 && cnt > 0) {
      --cnt;
      res.code[cnt] = alphabet[input % alphabet.size()];
      input /= alphabet.size();
    }
    // If the string remainder is only a's the input value is 0, but
    // we need to reach our length
    while (cnt > 0) {
      --cnt;
      res.code[cnt] = 'A';
    }
    if (input != 0) {
      // remainder is left so the input was too big
      return std::nullopt;
    }
    return res;
  }
  [[nodiscard]] constexpr auto to_int() const noexcept -> std::uint16_t {
    constexpr auto impl{ [](const string_type& input) -> std::uint16_t {
      constexpr auto const_toupper{ [](char character) -> char {
        if ('a' <= character && character <= 'z') {
          return static_cast<char>(character - ('a' - 'A'));
        }
        return character;
      } };
      std::uint16_t res = 0;
      static_assert(alphabet.size() == 26);
      for (auto const& character : input) {
        res *= alphabet.size();
        res += const_toupper(character) - 'A';
      }
      return res;
    } };
    if (outside_spec) {
      return offset + impl(code);
    }
    return impl(code);
  }
  constexpr auto operator==(species const& rhs) const noexcept -> bool = default;

  static constexpr std::string_view alphabet{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
  static constexpr std::uint16_t offset{ alphabet.size() * alphabet.size() * alphabet.size() };
};
inline constexpr auto atlantic_cod{ species{ .code{ "COD" } } };
inline constexpr auto atlantic_herring{ species{ .code{ "HER" } } };
inline constexpr auto haddock{ species{ .code{ "HAD" } } };
inline constexpr auto european_sprat{ species{ .code{ "SPR" } } };
inline constexpr auto whiting{ species{ .code{ "WHG" } } };
inline constexpr auto blue_whiting{ species{ .code{ "WHB" } } };
inline constexpr auto red_gurnard{ species{ .code{ "GUR" } } };
inline constexpr auto boarfish{ species{ .code{ "BOC" } } };
inline constexpr auto mackerels_nei{ species{ .code{ "MAX" } } };
inline constexpr auto european_hake{ species{ .code{ "HKE" } } };
inline constexpr auto pollack{ species{ .code{ "POL" } } };
inline constexpr auto atlantic_halibut{ species{ .code{ "HAL" } } };
inline constexpr auto atlantic_horse_mackerel{ species{ .code{ "HOM" } } };
inline constexpr auto norway_pout{ species{ .code{ "NOP" } } };
inline constexpr auto greater_argentine{ species{ .code{ "ARU" } } };
inline constexpr auto atlantic_salmon{ species{ .code{ "SAL" } } };
// outside spec
inline constexpr auto incomplete{ species{ .outside_spec = true, .code{ "ICP" } } };
inline constexpr auto damaged{ species{ .outside_spec = true, .code{ "DMG" } } };
inline constexpr auto single{ species{ .outside_spec = true, .code{ "SNG" } } };
inline constexpr auto double_{ species{ .outside_spec = true, .code{ "DBL" } } };
inline constexpr auto unsure{ species{ .outside_spec = true, .code{ "UNS" } } };
inline constexpr auto empty{ species{ .outside_spec = true, .code{ "EMP" } } };
inline constexpr auto gigolo{ species{ .outside_spec = true, .code{ "GIG" } } };  // example half of a fish in an image
inline constexpr auto garbage{ species{ .outside_spec = true, .code{ "GAR" } } };

namespace test {
// notice that if you pass into from_int an integer outside the scope of the parser a nullopt is returned
static_assert(species::from_int(std::numeric_limits<std::uint16_t>::max()) == std::nullopt);

// 25 is Z and 27 is the alphabet size
static_assert(species{ .outside_spec = false, .code{ "ZZZ" } }.to_int() == (25 * 26 + 25) * 26 + 25);  // 18925
static_assert(species::from_int(species::offset - 1) == species{ .outside_spec = false, .code{ "ZZZ" } });

static_assert(atlantic_cod.to_int() == 1719);
static_assert(atlantic_cod == species::from_int(atlantic_cod.to_int()));
static_assert(red_gurnard.to_int() == 4593);
static_assert(red_gurnard == species::from_int(red_gurnard.to_int()));
static_assert(damaged.to_int() == 19922);
static_assert(damaged == species::from_int(damaged.to_int()));
static_assert(gigolo.to_int() == 21846);
static_assert(gigolo == species::from_int(gigolo.to_int()));
static_assert(empty == species::from_int(empty.to_int()));
static_assert(empty.to_int() == 20607);

static_assert(species::from_3a("AZS")->to_int() == 668);
static_assert(species::from_int(668)->code == stx::basic_fixed_string<char, 3>("AZS"));

}  // namespace test

}  // namespace fao

namespace si = mp_units::si;
namespace isq = mp_units::isq;

struct item;

/// \brief creates a new item with a new id and timestamp
[[nodiscard]] auto make() -> item;
[[nodiscard]] auto make(pcg_extras::seed_seq_from<std::random_device>& seed_source) -> item;

/// \struct item
/// \brief given attributes of an item
struct item {
  using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

  [[nodiscard]] static auto from_json(std::string_view json) -> std::expected<item, glz::error_ctx>;
  [[nodiscard]] auto to_json() const -> std::expected<std::string, glz::error_ctx>;
  [[nodiscard]] auto id() const -> std::string;

  // ids
  std::optional<uuids::uuid> item_id{ std::nullopt };
  std::optional<uuids::uuid> batch_id{ std::nullopt };
  std::optional<std::string> barcode{ std::nullopt };
  std::optional<std::string> qr_code{ std::nullopt };

  // type
  std::optional<details::category_e> category{ std::nullopt };
  std::optional<fao::species> fao_species{ std::nullopt };
  std::optional<std::string> sub_type{ std::nullopt };  // any specification of the given type

  // dimensions
  // si::kilogram kg;
  using milligram_64bit = mp_units::quantity<si::milli<si::gram>, int64_t>;
  std::optional<milligram_64bit> item_weight{ std::nullopt };
  std::optional<milligram_64bit> target_weight{ std::nullopt };
  std::optional<milligram_64bit> min_weight{ std::nullopt };
  std::optional<milligram_64bit> max_weight{ std::nullopt };

  using millimetre_64bit = mp_units::quantity<si::milli<si::metre>, uint64_t>;
  std::optional<millimetre_64bit> length{ std::nullopt };
  std::optional<millimetre_64bit> width{ std::nullopt };
  std::optional<millimetre_64bit> height{ std::nullopt };

  std::optional<mp_units::quantity<mp_units::square(si::milli<si::metre>), uint64_t>> area{ std::nullopt };

  std::optional<mp_units::quantity<mp_units::cubic(si::milli<si::metre>), uint64_t>> volume{ std::nullopt };
  std::optional<mp_units::quantity<mp_units::si::degree_Celsius, double>> temperature{ std::nullopt };
  std::optional<mp_units::quantity<mp_units::angular::degree, double>> angle{ std::nullopt };

  // attributes
  std::optional<details::color> color{ std::nullopt };
  std::optional<details::quality_e> quality{ std::nullopt };
  std::optional<time_point> entry_timestamp{ std::nullopt };
  std::optional<time_point> production_date{ std::nullopt };
  std::optional<time_point> expiration_date{ std::nullopt };
  std::optional<time_point> last_exchange{
    std::nullopt
  };  // Last time that the item was exchanged between systems, i.e. parsed from json
  std::optional<std::string> description{ std::nullopt };
  std::optional<details::supplier> supplier{ std::nullopt };
  std::optional<details::destination> destination{ std::nullopt };

  // item may contain other items
  std::optional<std::vector<item>> items{ std::nullopt };

  [[nodiscard]] auto operator==(item const& rhs) const noexcept -> bool = default;
};

}  // namespace tfc::ipc::item
