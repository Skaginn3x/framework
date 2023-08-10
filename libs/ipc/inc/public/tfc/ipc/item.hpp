#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <units/generic/angle.h>
#include <units/isq/si/area.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/volume.h>
#include <glaze/core/common.hpp>

#include <tfc/stx/basic_fixed_string.hpp>

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
};
struct supplier {
  std::string name{};
  std::string contact_info{};
  std::string origin{};
};
}  // namespace details

// Food and Agriculture Organization of the United Nations
// Reference: https://www.fao.org/fishery/en/collection/asfis/en
namespace fao {
struct species {
  bool outside_spec{ false };  // if struct is used for outside the specification, represented with `!` as first character
  tfc::stx::basic_fixed_string<char, 3> code{};

  static constexpr auto from_int(std::uint16_t input) -> species {
    species res{};
    if (input >= offset) {
      res.outside_spec = true;
      input -= offset;
    }
    unsigned int cnt{ 3 };
    while (input > 0 && cnt > 0) {
      res.code[--cnt] = alphabet[input % alphabet.size()];
      input /= alphabet.size();
    }
    return res;
  }
  [[nodiscard]] inline constexpr auto to_int() const noexcept -> std::uint16_t {
    static constexpr auto impl{ [](auto& input) -> std::uint16_t {
      static constexpr auto const_toupper{ [](char character) -> char {
        if ('a' <= character && character <= 'z') {
          return static_cast<char>(character - ('a' - 'A'));
        }
        return character;
      } };
      std::uint16_t res = 0;
      static_assert(alphabet.size() == 27);
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
  constexpr auto operator==(species const& rhs) const noexcept { return outside_spec == rhs.outside_spec && code == rhs.code; }

private:
  static constexpr std::string_view alphabet{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ!" };  // note the ending !
  // This offset is kind of ridiculous, as in the - 1
  // todo this can overlap !!!
  static constexpr std::uint32_t offset{ (alphabet.size() - 1) * (alphabet.size() - 1) * (alphabet.size() - 1) };
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
static_assert(atlantic_cod.to_int() == 1839);
static_assert(atlantic_cod == species::from_int(atlantic_cod.to_int()));
static_assert(red_gurnard.to_int() == 4931);
static_assert(red_gurnard == species::from_int(red_gurnard.to_int()));
static_assert(damaged.to_int() == 20093);
static_assert(damaged == species::from_int(damaged.to_int()));
static_assert(gigolo.to_int() == 22172);
static_assert(gigolo == species::from_int(gigolo.to_int()));

}  // namespace test

}  // namespace fao

namespace si = units::isq::si;

/// \struct item
/// \brief given attributes of an item
struct item {
  static constexpr auto make() {
    // todo generate uuid
    // todo generate entry time point
    return item{};
  }

  // ids
  std::optional<std::string> item_id{ std::nullopt };
  std::optional<std::string> batch_id{ std::nullopt };
  std::optional<std::string> barcode{ std::nullopt };
  std::optional<std::string> qr_code{ std::nullopt };

  std::optional<details::category_e> category{ std::nullopt };
  std::optional<fao::species> fao_species{ std::nullopt };
  std::optional<std::string> sub_type{ std::nullopt };  // any specification of the given type
  // dimensions
  std::optional<si::mass<si::milligram, std::uint64_t>> item_weight{ std::nullopt };
  std::optional<si::mass<si::milligram, std::uint64_t>> target_weight{ std::nullopt };
  std::optional<si::mass<si::milligram, std::uint64_t>> min_weight{ std::nullopt };
  std::optional<si::mass<si::milligram, std::uint64_t>> max_weight{ std::nullopt };
  std::optional<si::length<si::millimetre, std::uint64_t>> length{ std::nullopt };
  std::optional<si::length<si::millimetre, std::uint64_t>> width{ std::nullopt };
  std::optional<si::length<si::millimetre, std::uint64_t>> height{ std::nullopt };
  std::optional<si::area<si::square_millimetre, std::uint64_t>> area{ std::nullopt };
  std::optional<si::volume<si::cubic_millimetre, std::uint64_t>> volume{ std::nullopt };
  // TODO temperature in celsius
  std::optional<units::angle<units::degree, double>> angle{ std::nullopt };

  // attributes
  std::optional<details::color> color{ std::nullopt };
  std::optional<details::quality_e> quality{ std::nullopt };
  std::optional<std::chrono::system_clock::time_point> entry{ std::nullopt };  // when did item enter system
  std::optional<std::chrono::system_clock::time_point> production_date{ std::nullopt };
  std::optional<std::chrono::system_clock::time_point> expiration_date{ std::nullopt };
  std::optional<std::string> description{ std::nullopt };
  std::optional<details::supplier> supplier{ std::nullopt };

  // item may contain other items
  std::optional<std::vector<item>> items{};
};

}  // namespace tfc::ipc::item


namespace glz {

template<>
struct meta<tfc::ipc::item::details::category_e> {
  using enum tfc::ipc::item::details::category_e;
  static constexpr auto name{ "item_category" };
  static constexpr auto value{ glz::enumerate("unknown", unknown,
                                              "box", box,
                                              "tub", tub,
                                              "bag", bag,
                                              "pallet", pallet,
                                              "fish", fish,
                                              "meat", meat,
                                              "poultry", poultry,
                                              "ice", ice
                                              ) };

};
template<>
struct meta<tfc::ipc::item::details::quality_e> {
  using enum tfc::ipc::item::details::quality_e;
  static constexpr auto name{ "item_quality" };
  static constexpr auto value{ glz::enumerate("unknown", unknown,
                                              "inferior", inferior,
                                              "weak", weak,
                                              "ordinary", ordinary,
                                              "exceptional", exceptional,
                                              "superior", superior
                                              ) };

};
template<>
struct meta<tfc::ipc::item::details::color> {
  using type = tfc::ipc::item::details::color;
  static constexpr auto name{ "item_color" };
  static constexpr auto value{ glz::object("red", &type::red, "Red value 0-255"
                                           "green", &type::green, "Green value 0-255"
                                           "blue", &type::blue, "Blue value 0-255"
                                          ) };

};
template<>
struct meta<tfc::ipc::item::details::supplier> {
  using type = tfc::ipc::item::details::supplier;
  static constexpr auto name{ "item_supplier" };
  static constexpr auto value{ glz::object("name", &type::name, "Company name"
                                           "contact_info", &type::contact_info, "Company contact information"
                                           "origin", &type::origin, "Company country"
                                           ) };

};
template<>
struct meta<tfc::ipc::item::fao::species> {
  using type = tfc::ipc::item::fao::species;
  static constexpr auto name{ "fao_species" };
  static constexpr auto value{ glz::object("code", &type::code, "3 letter food and agriculture organization code"
                                           "outside_spec", &type::outside_spec, "Code is not according to FAO"
                                           ) };

};


template<>
struct meta<tfc::ipc::item::item> {
  using type = tfc::ipc::item::item;
  static constexpr auto name{ "ipc_item" };
  static constexpr auto value{ glz::object("id", &type::item_id, "Unique id of this item"
                                           "batch_id", &type::batch_id, "Unique id of this batch"
                                           ) };

};


}
