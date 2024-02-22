#pragma once

#include <glaze/core/common.hpp>

#include <tfc/ipc/item.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>
#include <tfc/utils/uuids_glaze_meta.hpp>

namespace glz {

template <>
struct meta<tfc::ipc::item::details::category_e> {
  using enum tfc::ipc::item::details::category_e;
  static constexpr auto name{ "item_category" };
  // clang-format off
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
  // clang-format on
};
template <>
struct meta<tfc::ipc::item::details::quality_e> {
  using enum tfc::ipc::item::details::quality_e;
  static constexpr auto name{ "item_quality" };
  // clang-format off
  static constexpr auto value{ glz::enumerate("unknown", unknown,
                                              "inferior", inferior,
                                              "weak", weak,
                                              "ordinary", ordinary,
                                              "exceptional", exceptional,
                                              "superior", superior
                                              ) };
  // clang-format on
};
template <>
struct meta<tfc::ipc::item::details::color> {
  using type = tfc::ipc::item::details::color;
  static constexpr auto name{ "item_color" };
  // clang-format off
  static constexpr auto value{ glz::object("red", &type::red, "Red value 0-255",
                                           "green", &type::green, "Green value 0-255",
                                           "blue", &type::blue, "Blue value 0-255"
                                          ) };
  // clang-format on
};
template <>
struct meta<tfc::ipc::item::details::supplier> {
  using type = tfc::ipc::item::details::supplier;
  static constexpr auto name{ "item_supplier" };
  // clang-format off
  static constexpr auto value{ glz::object("name", &type::name, "Supplier name",
                                           "contact_info", &type::contact_info, "Supplier contact information",
                                           "origin", &type::country, "Supplier country"
                                           ) };
  // clang-format on
};
template <>
struct meta<tfc::ipc::item::fao::species> {
  using type = tfc::ipc::item::fao::species;
  static constexpr auto name{ "fao_species" };
  // clang-format off
  static constexpr auto value{ glz::object("code", &type::code, "3 letter food and agriculture organization code",
                                           "outside_spec", &type::outside_spec, "Code is not according to FAO"
                                           ) };
  // clang-format on
};

template <>
struct meta<tfc::ipc::item::item> {
  using type = tfc::ipc::item::item;
  static constexpr auto name{ "ipc_item" };
  // clang-format off
  static constexpr auto value{ glz::object("id", &type::item_id, "Unique id of this item",
                                           "batch_id", &type::batch_id, "Unique id of this batch",
                                           "barcode", &type::barcode, "Unique barcode of this item",
                                           "qr_code", &type::qr_code, "Unique QR code of this item",
                                           "category", &type::category, "Item category",
                                           "fao_species", &type::fao_species, "Food and agriculture organization species code",
                                           "sub_type", &type::sub_type, "More specific type related information",
                                           "item_weight", &type::item_weight, "Weight of item",
                                           "target_weight", &type::target_weight, "Presumed weight of item",
                                           "min_weight", &type::min_weight, "Minimum acceptable weight of item",
                                           "max_weight", &type::max_weight, "Maximum acceptable weight of item",
                                           "length", &type::length, "Length of item",
                                           "width", &type::width, "Width of item",
                                           "height", &type::height, "Height of item",
                                           "area", &type::area, "Area of item",
                                           "volume", &type::volume, "Volume of item",
//                                           "temperature", &type::temperature, "Temperature in celsius",
                                           "angle", &type::angle, "Angle of item in its place",
                                           "color", &type::color, "RGB color value",
                                           "quality", &type::quality, "Quality/grade of item",
                                           "entry_timestamp", &type::entry_timestamp, "First entry timestamp of item appearing in system",
                                           "production_date", &type::production_date, "Production date of item",
                                           "expiration_date", &type::expiration_date, "Expiration date of item",
                                           "last_exchange", &type::last_exchange, "Last time item changed hands",
                                           "description", &type::description, "Description of item, some kind of metadata",
                                           "supplier", &type::supplier, "Supplier information of item",
//                                           "destination", &type::destination, "Routing destination of item",
                                           "items", &type::items, "List of owning items, like tub of 100 fishes"
                                           ) };
  // clang-format on
};

}  // namespace glz
