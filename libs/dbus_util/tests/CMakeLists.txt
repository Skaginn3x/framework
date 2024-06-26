
find_package(ut CONFIG REQUIRED)

add_executable(dbus_util_test dbus_util_test.cpp)

target_link_libraries(dbus_util_test PRIVATE Boost::ut tfc::logger tfc::base tfc::dbus_util)

add_test(NAME dbus_util_test COMMAND dbus_util_test)

add_executable(dbus_compile_tests dbus_compile_tests.cpp)

target_link_libraries(dbus_compile_tests PRIVATE Boost::ut tfc::logger tfc::base tfc::dbus_util)

add_test(NAME dbus_compile_tests COMMAND dbus_compile_tests)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
