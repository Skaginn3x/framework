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
#include <tfc/dbus/match_rules.hpp>
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

template <typename storage_t>
struct config_testable : public tfc::confman::config<storage_t> {
  using tfc::confman::config<storage_t>::config;
  ~config_testable() {
    std::error_code ignore{};
    std::filesystem::remove(this->file(), ignore);
  }
};

struct instance {
  boost::asio::io_context ctx{};
  std::shared_ptr<sdbusplus::asio::connection> dbus{
    std::make_shared<sdbusplus::asio::connection>(ctx, tfc::dbus::sd_bus_open_system())
  };
  std::string const key{ "bar" };
  std::string const service_name{ tfc::dbus::make_dbus_process_name() };
  // duplicate from config_dbus_client.cpp
  std::filesystem::path const interface_path{ tfc::dbus::make_dbus_path(key) };
  std::string const interface_name{ tfc::confman::detail::dbus::interface };
  storage const storage_{};
  config_testable<storage> config{ dbus, key, storage{ storage_ } };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);
  using tfc::confman::detail::dbus::property_value_name;

  "default values"_test = [] {
    instance const test{ .storage_ = {
                             .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } } };
    ut::expect(1 == test.config->a);
    ut::expect(2 == test.config->b);
    ut::expect("bar" == test.config->c);
    ut::expect(1 == test.config.value().a);
    ut::expect(2 == test.config.value().b);
    ut::expect("bar" == test.config.value().c);
  };

  "to json"_test = [] {
    instance const test{ .storage_ = {
                             .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } } };
    auto const json_str{ test.config.string() };
    glz::json_t json{};
    std::ignore = glz::read_json(json, json_str);
    ut::expect(static_cast<int>(json["a"].get<double>()) == 1);
    ut::expect(static_cast<int>(json["b"].get<double>()) == 2);
    ut::expect(json["c"].get<std::string>() == "bar");
  };

  "change value internally"_test = [] {
    instance test{ .storage_ = {
                       .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } } };
    uint8_t a_called_once{};
    test.config->a.observe([&a_called_once](int new_val, int old_val) {
      ut::expect(new_val == 3);
      ut::expect(old_val == 1);
      a_called_once++;
    });
    test.config.make_change()->a = 3;
    ut::expect(a_called_once == 1);
  };

  "from json"_test = [] {
    instance test{};
    glz::json_t json{};
    json["a"] = 11;
    json["b"] = 22;
    json["c"] = "meeoow";
    test.config.from_string(glz::write_json(json));
    ut::expect(11 == test.config->a);
    ut::expect(22 == test.config->b);
    ut::expect("meeoow" == test.config->c);
  };

  "from json calls observers"_test = [] {
    instance test{ .storage_ = {
                       .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } } };

    uint32_t c_called{};
    test.config->c.observe([&c_called](std::string const& new_val, std::string const& old_val) {
      ut::expect("meeoow" == new_val);
      ut::expect("bar" == old_val);
      c_called++;
    });

    glz::json_t json{};
    json["a"] = 11;
    json["b"] = 22;
    json["c"] = "meeoow";
    test.config.from_string(glz::write_json(json));

    ut::expect(1 == c_called);
  };

  "integration get_config"_test = [] {
    instance test{ .storage_ = {
                       .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } } };
    test.dbus->request_name(test.service_name.c_str());

    uint32_t called{};
    sdbusplus::asio::getProperty<std::string>(
        *test.dbus, test.service_name, test.interface_path.string(), test.interface_name, std::string{ property_value_name },
        [&called]([[maybe_unused]] std::error_code err, [[maybe_unused]] std::string prop) {
          ut::expect(!err) << err.message();
          called++;
          glz::json_t json{};
          std::ignore = glz::read_json(json, prop);
          ut::expect(static_cast<int>(json["a"].get<double>()) == 1);
          ut::expect(static_cast<int>(json["b"].get<double>()) == 2);
          ut::expect(json["c"].get<std::string>() == "bar");
        });

    test.ctx.run_for(std::chrono::milliseconds(10));
    ut::expect(called == 1);
  };

  "integration set_config"_test = [] {
    instance test{ .storage_ = {
                       .a = observable<int>{ 1 }, .b = observable<int>{ 2 }, .c = observable<std::string>{ "bar" } } };
    test.dbus->request_name(test.service_name.c_str());

    uint32_t a_called{};
    test.config->a.observe([&a_called](int new_a, int old_a) {
      a_called++;
      ut::expect(new_a == 11);
      ut::expect(old_a == 1);
    });

    uint32_t called{};
    sdbusplus::asio::setProperty<std::string>(
        *test.dbus, test.service_name, test.interface_path.string(), test.interface_name, std::string{ property_value_name },
        std::string{ R"({"a":11,"b":12,"c":"a"})" }, [&called, &test]([[maybe_unused]] std::error_code err) {
          if (err) {
            fmt::print(stderr, "Set property error: '{}'", err.message());
          }
          called++;
          ut::expect(test.config->a == 11);
          ut::expect(test.config->b == 12);
          ut::expect(test.config->c == "a");
        });

    test.ctx.run_for(std::chrono::milliseconds(10));
    ut::expect(called == 1);
    ut::expect(a_called == 1);
  };

  "integration await property changed internally"_test = [] {
    instance test{};

    uint32_t called{};
    auto match = fmt::format("type='signal',member='PropertiesChanged',path='{}'", test.interface_path.string());
    sdbusplus::bus::match_t const awaiter{ *test.dbus, match,
                                           [&called]([[maybe_unused]] sdbusplus::message::message& msg) { called++; } };

    test.config.make_change()->a = 42;

    test.ctx.run_for(std::chrono::milliseconds(10));
    ut::expect(called == 1);
  };

  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
