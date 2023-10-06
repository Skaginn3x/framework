# Time For Change 

A C++ microservices framework which purpose is to simplify development of applications. 
How would one ask, the framework offers an API which simplifies communication between processes and runtime configuration storage.

**Important** this repository both contains the library API of the framework along with already made use cases(executables) like:
- IPC ruler
- tfcctl (nice utility to test with)
- Ethercat master
- MQTT-bridge
- Operation mode (start, stop, emergency, fault, etc.)
- etc.

The IPC ruler distinguishes from every other executable, as it is needed to use the communication layer. 
So this is the one daemon that you would need to start to make your process run.

## Prerequisites

We have many dependencies on other open source software stacks. 
The main being [boost asio](https://github.com/boostorg/asio), [zeromq](https://github.com/zeromq), [sdbus](https://github.com/systemd/systemd/), [json(glaze)](https://github.com/stephenberry/glaze/).
These components will be referred to multiple times, so you might need to look up their documentation.

## API

### Communication layer
The communication layer uses simple predefined message format.
The following message format are currently supported: 
```C++
enum struct filter_e : std::uint8_t {
  _bool = 1,
  _int64_t = 2,
  _uint64_t = 3,
  _double_t = 4,
  _string = 5,
  _json = 6,
};
```
It is worth to mention that json values are passed as string through the communication channel.

What makes this communication layer **unique** is the simplicity to connect those channels, you are able to change 
connections during runtime, without restarting any process. 

The terminology used in the communication layer is `signals` for owner of information and `slot` for connecting to `signal`. 
And on top of that this mechanism works like one-to-many pattern. 

To gain greater depth of understanding please refer to the [IPC docs](https://skaginn3x.github.io/framework/design/ipc.html).

#### Example

```C++
#include <boost/asio.hpp>
#include <tfc/ipc.hpp>
using asio = boost::asio;
struct my_application {
  asio::io_context& ctx;
  tfc::ipc::bool_signal{ ctx, "name_of_signal", "optional description of signal" };
  tfc::ipc::bool_slot{ ctx, "name_of_slot", "optional description of slot", [](bool new_value){} };
};
```

### Configuration storage

Each individual part of a software can own a config instance object. 
The config is stored into a json file located default in `/etc/tfc/` but can be overwritten with the environment variable `CONFIGURATION_DIRECTORY`.

#### Config structure
Currently, you would declare your struct for example and its according projection to json with [glaze](https://github.com/stephenberry/glaze/). For example:
```C++
#include <chrono>
#include <glaze/glaze.hpp>
#include <units/isq/si/si.h>
#include <tfc/confman/observable.hpp>
struct storage {
  std::chrono::milliseconds drop_time{};
  units::quantity<units::isq::si::dim_speed, units::isq::si::metre_per_second, int32_t> target_speed{};
  tfc::confman::observable<int> state{};
  struct glaze {
    static constexpr auto value{ glz::object("drop_time", &storage::drop_time, "drop time description ...",
                                             "target_speed", &storage::target_speed, "target speed description ...",
                                             "state", &storage::state, "state description ...") };
    static constexpr auto name{ "name of this storage struct" };
  };
};
```
This struct would be wrapped with our confman class.
```C++
#include <boost/asio.hpp>
#include <fmt/chrono.hpp>
#include <tfc/confman.hpp>
using asio = boost::asio;
struct my_application {
  my_application(asio::io_context& ctx) : ctx{ ctx } {
    config->state.observe([](int new_value, int old_value){
      // here you can get a callback whenever this one variable is changed
    });
  }
  asio::io_context& ctx;
  tfc::confman::config<storage> config{ ctx, "unique config key to this process" };
  void using_config() {
    fmt::print("drop time is {}", config->drop_time);
  }
};
```

#### Interface to change the config
There are currently two ways of changing the config, one being through a `dbus` API and the second being changing the file directly.

##### dbus API
**Disclaimer the naming scheme can change in the future.**

The interface name is for example `com.skaginn3x.config.etc.tfc.operation_mode.def.state_machine` which owns a dbus property of type
struct of two strings,
```
{
string,
string
}
```
The first string is the config in json format, the second string is the json schema of the configuration.
To change the config you can write to the property and the process will update its config file.

There exist many dbus frontends like [D-Feet](https://wiki.gnome.org/Apps/DFeet) which can help with understanding any dbus related API.

##### Changing file

User can change the file on the system and the process will get notification via `inotify`.

## Executables

Todo


## Start hacking

### Package management

To begin with executables will be statically linked to third party dependencies but shared linking to libc and libc++/libstdc++.

[vcpkg](https://github.com/microsoft/vcpkg) will be used to declare, build dependencies. To set up on Arch Linux 
```bash
yay -S vcpkg
sudo gpasswd -a $USER vcpkg
sudo pacman -S libc++ lld llvm base-devel ccache autoconf-archive meson gperf
# for building documentation
sudo pacman -S python-sphinx_rtd_theme python-myst-parser python-breathe
# for packaging
sudo pacman -S rpm-tools
# for cross compiling
sudo pacman -S aarch64-linux-gnu-gcc aarch64-linux-gnu-binutils aarch64-linux-gnu-gdb aarch64-linux-gnu-glibc
# relogin or $ newgrp vcpkg in terminal or su <username>
```
Please note the output of installing vcpkg:
```bash
    "VCPKG_ROOT" is set to "/opt/vcpkg"
    "VCPKG_DOWNLOADS" is set to "/var/cache/vcpkg"
    To cooperate with CMake, add "-DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

**please note that for the first release (first deployed product) it will be required to fixate the vcpkg dependencies version**

### Build system

Everything will be built on CMake and using features from 3.23+ version.

# Install with vcpkg

Add to your `vcpkg-configuration.json` file the following section:

```json
{
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/skaginn3x/framework",
      "baseline": "7631f756bf5bcc010469b513ee242d932f72835a",
      "packages": [
        "sdbusplus",
        "tfc-framework"
      ]
    }
  ]
}
```

# Copyright
Copyright 2023 Skaginn 3X ehf
