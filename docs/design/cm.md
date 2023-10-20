# Configuration manager ( confman )

## Description 

A library component responsible for tracking current configuration
accepting configuration changes and notifing parties of configuration
changes.

## Features
- Log configuration changes
- Make configuration management available on dbus ( cockpit f.e. )
- Services declare configuration fields, type, limits and defaults to configuration manager over dbus.
- Configuration to be persistent between reboots
- Eazy backup of configuration
- Eazy restore of configureation
- Configuration retention

## Play confman interface
```cpp
#include <chrono>
#include <cstdint>
#include <string>

#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

struct simple_config {
  int a{};
  std::string b{};
  tfc::confman::observable<bool> c{};
  std::vector<int> d{};
  std::chrono::nanoseconds sec{};
  mp_units::quantity<mp_units::si::deci<mp_units::si::ampere>> amper{};
  struct glaze {
    using type = simple_config;
    static constexpr auto value{ glz::object(
        "a",
        &type::a,
        tfc::json::schema{ .description = "A description", .minimum = 100, .maximum = 300 },
        "b",
        &type::b,
        "c",
        &type::c,
        "C description",
        "d",
        &type::d,
        "D description",
        "sec",
        &type::sec,
        tfc::json::schema{ .description = "Sec description", .minimum = 1000, .maximum = 30000 },
        "amper",
        &type::amper,
        "Amper description") };
    static constexpr auto name{ "simple_config" };
  };
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  tfc::confman::config<simple_config> const config{ ctx, "key" };
  config->c.observe(
      [](bool new_value, bool old_value) { fmt::print("new value: {}, old value: {}\n", new_value, old_value); });

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  ctx.run();
  return 0;
}
```


## Inital implementation of storage and backup
Will utilize file level json files.
Backups will be implemented by joining the relevant json files
and downloading a single large file.

## Log changes to configuration
In the first iteration this can be as simple
as printing the current json and the json
to bo applied.

```
==================> Configuration changed from 
JSON BLOB
==================< TO 
JSON BLOB
==================<>
```

## Confman description of underlying communication
Confman exposes a dbus interface that can be used to
change configuration and to get the current configuration.


## Configuration retention policy
Confman will keep a configurable minumum number of backups of the configuration.
Suggested default is 4, this can be adjusted by an environment variable.
`TFC_CONFMAN_MIN_RETENTION_COUNT`.
Confman will also keep all backups that are newer then some days. Suggested
default is 30 days. This can be adjusted by an environment variable.
`TFC_CONFMAN_MIN_RETENTION_DAYS`.

Confman shall only act upon these rules when writing new configuration
and no routine checks shall be performed to remove old backups.

### Configuration naming rules
Confman will name configuration files with the following naming scheme:
`<configuration_name>_<uuid>.json`. Where uuid is specified by
[https://datatracker.ietf.org/doc/html/rfc4122](https://datatracker.ietf.org/doc/html/rfc4122)