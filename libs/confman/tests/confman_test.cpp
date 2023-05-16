#include <filesystem>

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/program_options.hpp>
#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/property.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/sd_bus.hpp>
#include <tfc/dbus/sdbusplus_meta.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/progbase.hpp>

namespace asio = boost::asio;
namespace ut = boost::ut;
using ut::operator""_test;
using ut::operator/;
using tfc::confman::observable;

struct storage {
  observable<int> a{ 33 };
  observable<int> b{ 44 };
  observable<std::string> c{ "c" };
};

template <>
struct glz::meta<storage> {
  static constexpr auto value{ glz::object("a", &storage::a, "b", &storage::b, "c", &storage::c) };
};

namespace bpo = boost::program_options;

// static std::string replace_all(std::string const& input, std::string_view what, std::string_view with) {
//   std::size_t count{};
//   std::string copy{ input };
//   for (std::string::size_type pos{}; std::string::npos != (pos = copy.find(what.data(), pos, what.length()));
//        pos += with.length(), ++count) {
//     copy.replace(pos, what.length(), with.data(), with.length());
//   }
//   return copy;
// }

template <typename storage_t>
class config_testable : public tfc::confman::config<storage_t> {
public:
  using tfc::confman::config<storage_t>::config;

  ~config_testable() {
    std::error_code ignore{};
    std::filesystem::remove(this->file(), ignore);
  }
};

auto main(int argc, char** argv) -> int {
  auto desc{ tfc::base::default_description() };
  bool run_slow{};
  desc.add_options()("slow,s", bpo::bool_switch(&run_slow)->default_value(false));
  tfc::base::init(argc, argv, desc);

  boost::asio::io_context ignore{};

  std::string const key{ "bar" };

  auto interface_path{ std::filesystem::path{ tfc::dbus::make_dbus_path("") } /
                       tfc::base::make_config_file_name(key, "").string().substr(1) };
  auto interface_name{ interface_path.string().substr(1) };
  std::replace(interface_name.begin(), interface_name.end(), '/', '.');

  using tfc::confman::detail::config_property;
  using tfc::confman::detail::dbus::property_name;

  "default values"_test = [&] {
    config_testable<storage> const conf{
      ignore, key, storage{ .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } }
    };
    ut::expect(1 == conf->a);
    ut::expect(2 == conf->b);
    ut::expect("bar" == conf->c);
    ut::expect(1 == conf.value().a);
    ut::expect(2 == conf.value().b);
    ut::expect("bar" == conf.value().c);
  };

  "to json"_test = [&] {
    config_testable<storage> const conf{
      ignore, key, storage{ .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } }
    };
    auto const json_str{ conf.string() };
    glz::json_t json{};
    std::ignore = glz::read_json(json, json_str);
    ut::expect(static_cast<int>(json["a"].get<double>()) == 1);
    ut::expect(static_cast<int>(json["b"].get<double>()) == 2);
    ut::expect(json["c"].get<std::string>() == "bar");
  };

  "change value internally"_test = [&] {
    config_testable<storage> conf{
      ignore, key, storage{ .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } }
    };
    uint8_t a_called_once{};
    conf->a.observe([&a_called_once](int new_val, int old_val) {
      ut::expect(new_val == 3);
      ut::expect(old_val == 1);
      a_called_once++;
    });
    conf.make_change()->a = 3;
    ut::expect(a_called_once == 1);
  };

  "from json"_test = [&] {
    config_testable<storage> conf{ ignore, key };
    glz::json_t json{};
    json["a"] = 11;
    json["b"] = 22;
    json["c"] = "meeoow";
    conf.from_string(glz::write_json(json));
    ut::expect(11 == conf->a);
    ut::expect(22 == conf->b);
    ut::expect("meeoow" == conf->c);
  };

  "from json calls observers"_test = [&] {
    config_testable<storage> conf{
      ignore, key, storage{ .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } }
    };

    uint32_t c_called{};
    conf->c.observe([&c_called](std::string const& new_val, std::string const& old_val) {
      ut::expect("meeoow" == new_val);
      ut::expect("bar" == old_val);
      c_called++;
    });

    glz::json_t json{};
    json["a"] = 11;
    json["b"] = 22;
    json["c"] = "meeoow";
    conf.from_string(glz::write_json(json));

    ut::expect(1 == c_called);
  };

  "integration get_config"_test = [&] {
    boost::asio::io_context ctx{};
    sdbusplus::asio::connection dbus{ ctx, tfc::dbus::sd_bus_open_system() };
    config_testable<storage> const conf{
      ctx, key, storage{ .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } }
    };

    uint32_t called{};
    sdbusplus::asio::getProperty<config_property>(
        dbus, interface_name, interface_path.string(), interface_name,
        std::string{ property_name.data(), property_name.size() },
        [&called]([[maybe_unused]] std::error_code err, [[maybe_unused]] config_property prop) {
          ut::expect(!err);
          called++;
          glz::json_t json{};
          std::ignore = glz::read_json(json, prop.value);
          ut::expect(static_cast<int>(json["a"].get<double>()) == 1);
          ut::expect(static_cast<int>(json["b"].get<double>()) == 2);
          ut::expect(json["c"].get<std::string>() == "bar");
        });

    ctx.run_for(std::chrono::milliseconds(10));
    ut::expect(called == 1);
  };

  "integration set_config"_test = [&] {
    boost::asio::io_context ctx{};
    sdbusplus::asio::connection dbus{ ctx, tfc::dbus::sd_bus_open_system() };
    config_testable<storage> const conf{
      ctx, key, storage{ .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } }
    };

    uint32_t a_called{};
    conf->a.observe([&a_called](int new_a, int old_a) {
      a_called++;
      ut::expect(new_a == 11);
      ut::expect(old_a == 1);
    });

    uint32_t called{};
    sdbusplus::asio::setProperty<config_property>(dbus, interface_name, interface_path.string(), interface_name,
                                                  std::string{ property_name.data(), property_name.size() },
                                                  config_property{ R"({"a":11,"b":12,"c":"a"})", "" },
                                                  [&called, &conf]([[maybe_unused]] std::error_code err) {
                                                    if (err) {
                                                      fmt::print(stderr, "Set property error: '{}'", err.message());
                                                    }
                                                    called++;
                                                    ut::expect(conf->a == 11);
                                                    ut::expect(conf->b == 12);
                                                    ut::expect(conf->c == "a");
                                                  });

    ctx.run_for(std::chrono::milliseconds(10));
    ut::expect(called == 1);
    ut::expect(a_called == 1);
  };

  "integration change_file"_test = [&] {
    boost::asio::io_context ctx{};
    config_testable<storage> const conf{ ctx, key };

    uint32_t a_called{};
    conf->a.observe([&a_called](int new_a, int) {
      a_called++;
      ut::expect(new_a == 27);
    });

    glz::json_t json{};
    std::ignore = glz::read_json(json, conf.string());
    json["a"] = 27;

    std::ignore = glz::write_file_json(json, tfc::base::make_config_file_name(key, "json"));

    ctx.run_for(std::chrono::milliseconds(10));
    ut::expect(a_called == 1);
  };

  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
