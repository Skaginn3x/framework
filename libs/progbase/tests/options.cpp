#include <boost/ut.hpp>
#include <boost/program_options.hpp>
#include <tfc/progbase.hpp>

auto main(int argc, char **argv) -> int {
  using namespace boost::ut;
  namespace bpo = boost::program_options;

  "exe_name"_test = []() {
    constexpr const char *argv_test[] = {"foo", "--noeffect", "true", nullptr};
    tfc::base::init(3, argv_test, tfc::base::default_description());
    expect(tfc::base::get_exe_name() == "foo");
  };

  "default_proc_name"_test = [&argc, &argv]() {
    tfc::base::init(argc, argv, tfc::base::default_description());
    expect(tfc::base::get_proc_name() == "def");
  };
  "proc_name"_test = []() {
    constexpr const char *argv_test[] = {"foo", "--id", "hello-world", nullptr};
    tfc::base::init(3, argv_test, tfc::base::default_description());
    expect(tfc::base::get_proc_name() == "hello-world");
  };

  "default_stdout_disabled"_test = [&argc, &argv]() {
    tfc::base::init(argc, argv, tfc::base::default_description());
    expect(!tfc::base::is_stdout_enabled());
  };
  "stdout_enabled"_test = []() {
    constexpr const char *argv_test[] = {"foo", "--stdout", "true", nullptr};
    tfc::base::init(3, argv_test, tfc::base::default_description());
    expect(tfc::base::is_stdout_enabled());
  };

  "default_noeffect_disabled"_test = [&argc, &argv]() {
    tfc::base::init(argc, argv, tfc::base::default_description());
    expect(!tfc::base::is_noeffect_enabled());
  };
  "noeffect_enabled"_test = []() {
    constexpr const char *argv_test[] = {"foo", "--noeffect", "true", nullptr};
    tfc::base::init(3, argv_test, tfc::base::default_description());
    expect(tfc::base::is_noeffect_enabled());
  };

  "custom_options"_test = []() {
    constexpr const char *argv_test[] = {"foo", "--bar", "value", nullptr};
    auto desc{tfc::base::default_description()};
    std::string bar;
    desc.add_options()("bar", bpo::value<std::string>(&bar), "Help text");
    tfc::base::init(3, argv_test, desc);
    expect(bar == "value");
    expect(tfc::base::get_map()["bar"].as<std::string>() == "value");
  };
}
