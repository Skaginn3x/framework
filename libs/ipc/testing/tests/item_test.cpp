#include <tfc/ipc/item.hpp>

#include <string>
#include <cstdint>
#include <fmt/core.h>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>

auto main(int, char**) -> int {
  namespace ut = boost::ut;

  using ut::operator""_test;
  using ut::expect;
  using ut::operator>>;
  using ut::fatal;

  namespace item = tfc::ipc::item;
  namespace fao = tfc::ipc::item::fao;

  "fao"_test = []() {
    static constexpr auto foo = fao::species::from_int(fao::atlantic_cod.to_int());
    fmt::print("foo is: {}\n", foo->code.view());
  };

  "make creates distinct id"_test = [] {
    auto item = item::make();
    auto item2 = item::make();
    expect(item.item_id != item2.item_id);
  };

  "make creates timestamp"_test = [] {
    auto item = item::make();
    expect(item.entry_timestamp.has_value());
  };

  "json"_test = [] {
    auto item = item::make();
    auto remake = item::item::from_json(item.to_json()).value();
    // last_exchange is updated when from_json is called
    expect(item.id() == remake.id());
    expect(item.entry_timestamp == remake.entry_timestamp);
  };
  "Fish species is transitive"_test = [] {
    for(std::uint16_t i = 0; i < 17576; i++){
      using tfc::ipc::item::fao::species;
      auto item_from_int = species::from_int(i);
      ut::expect(item_from_int.has_value());
      ut::expect(i == item_from_int->to_int()) << " i: " << i << " gen i: " << item_from_int->to_int() << " ";
      ut::expect(species::from_3a(item_from_int->code.view()).value() == item_from_int.value()) << " i: " << species::from_3a(item_from_int->code.view()).value().to_int() << " gen i: " << item_from_int->to_int() << " ";
    }
  };
  "full database verification"_test = [](){
    //TODO: Add static asserts here that verify the results from our species encoder
    // against our implementation in python
    ut::expect(true);
  };
  return 0;
}
