module;
#include <chrono>
#include <sdbusplus/sdbus.hpp>
#include <sdbusplus/asio/connection.hpp>
export module sdbus;

import std;

extern "C++" {
}


export namespace sdbusplus {
  using sdbusplus::asio::connection;
}
