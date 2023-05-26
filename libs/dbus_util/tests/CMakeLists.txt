
find_package(ut CONFIG REQUIRED)

add_executable(dbus_util_test dbus_util_test.cpp)

target_link_libraries(dbus_util_test PRIVATE Boost::ut tfc::logger tfc::base tfc::dbus_util)

add_test(
  NAME
        dbus_util_test
  COMMAND
        dbus_util_test
)
