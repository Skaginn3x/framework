
// Generate a test case for python comparing the C++ implementation of the item class

#include <fmt/format.h>
#include <fmt/printf.h>
#include <tfc/ipc/item.hpp>

int main() {
  for (int species_index = 0; species_index < 26 * 26 * 26; species_index++) {
    std::string code(tfc::ipc::item::fao::species::from_int(species_index)->code.data());
    fmt::println("        self.assertEqual(le.from_int_to_3a({}), \"{}\")", species_index, code);
    fmt::println("        self.assertEqual(le.from_3a_to_int(\"{}\"), {})", code, species_index);
  }
  return 0;
}