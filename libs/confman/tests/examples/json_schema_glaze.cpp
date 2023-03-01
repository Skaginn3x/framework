#include <string>
#include <array>
#include <glaze/glaze.hpp>
#include <boost/program_options.hpp>
#include <magic_enum.hpp>

#include "tfc/progbase.hpp"
#include "tfc/logger.hpp"

enum class Color {Red, Green, Blue};

template<>
    struct glz::meta<Color> {
  using enum Color;
  static constexpr std::string_view name = "Color";
  static constexpr auto value  = enumerate("Red", Red, "Green", Green, "Blue", Blue);
};

struct my_struct
{
  int i = 287;
  double d = 3.14;
  std::string hello = "Hello World";
  std::array<uint64_t, 3> arr = { 1, 2, 3 };
  Color clr = Color::Blue;

  struct glaze {
    using T = my_struct;
    static constexpr std::string_view name = "my_struct";
    static constexpr auto value = glz::object(
        "i", &T::i,
        "d", &T::d,
        "hello", &T::hello,
        "arr", &T::arr,
        "clr", &T::clr
    );
  };
};
struct my_struct2
{
  int i = 287;
  double d = 3.14;
  std::string hello = "Hello World";
  std::array<my_struct, 3> arr = {my_struct{}, my_struct{}, my_struct{} };

  struct glaze {
    using T = my_struct2;
    static constexpr auto value = glz::object(
        "i", &T::i,
        "d", &T::d,
        "hello", &T::hello,
        "arr", &T::arr
    );
  };
};

int main(int argc, char** argv){
  auto prog_desc{tfc::base::default_description()};
  tfc::base::init(argc, argv, prog_desc);

  using logger = tfc::logger::logger;
  using lvl = tfc::logger::lvl_e;

  logger a("Schema testing");
  a.log<lvl::info>("Let's get this party started");
  std::string schema = glz::write_json_schema<my_struct>();
  a.log<lvl::info>("\n{}", glz::prettify(schema));
}

/**
 * Flow, services define their config. This includes fields that
 * they need to know in order to function.
 *
 * When these services are started they register the schema
 * of their configuration object with the configuration manager.
 * This schema is generated at compile time by confman headers.
 *
 * Once confman has recived the schema it can use it for the following things.
 * - Inform front end of what configuration variables are available.
 * - Verify configuration changes from the front end.
 *
 *
 * Thoughts
 * - the confman service cannot read
 *   the json data into structs.
 *   the structs contained within
 *   are not know at confmans compile time.
 *
 *
 * */
