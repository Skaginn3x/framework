# API infrastructure

## What API technologies exist

There are numerous API techs existing for example:
- Websockets
- TCP/UDP sockets
- File sockets
- ZeroMQ, RabbitMQ, etc.
- D-bus

How to choose which one is the best is a great question which there is no answer to unless you can see into the future.

To begin with a D-bus API will be made for a couple of reasons, 
- it already implements a command line interface like `busctl`
- it has a native introspection client like `dfeet`
- [cockpit project](https://cockpit-project.org/) already integrates with d-bus

Conclusion, there are tools existing that do all the work providing even higher level APIs to the dbus API, 
which we do not need to make or maintain.  

## D-bus (This is the way)

Okay, this has been decided, whether this is a good idea or not it, we will see. To navigate through the implementations
available in `C++` a good summary is available [here](https://www.freedesktop.org/wiki/Software/DBusBindings/).

Candidates:

- [dbus-c++](https://salsa.debian.org/debian/dbus-cplusplus) 
  - This seems to be mature library written in `C++98` style using runtime polymorphism, integrates with other event loops.
  - Freedesktop warn that this is not actively being maintained.
- [sdbus-c++](https://github.com/Kistler-Group/sdbus-cpp) 
  - Systemd's implementation of dbus written in `C++17`.
  - Can statically link to systemd.
  - Actively maintained, not much but something.
  - Seems to integrate with other event loops.
- [dbus-asio](https://github.com/dbus-asio/dbus-asio)
  - Boost asio implementation of dbus.
  - Would be perfect, but it is not actively maintained or active in any way.
  - The library needs some work before it can be used, for example use outside `io_context` reference.

The `sdbus-c++` and `dbus-asio` will be analyzed further.

