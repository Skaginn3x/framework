add_executable(test_beckhoff beckhoff.cpp)
target_link_libraries(test_beckhoff tfc::base tfc::ec tfc::mock_ipc)

# Get access to private headers
get_property(tfc_ec_dirs TARGET tfc::ec PROPERTY INCLUDE_DIRECTORIES)
target_include_directories(test_beckhoff PRIVATE ${tfc_ec_dirs})

add_test(
  NAME
    test_beckhoff
  COMMAND
    test_beckhoff
)

add_executable(test_atv320_dbus_iface atv320-dbus-iface.cpp)
target_link_libraries(test_atv320_dbus_iface
  tfc::base
  tfc::ec
  tfc::mock_ipc
  tfc::stub_confman
  tfc::testing
  PkgConfig::SDBUSPLUS
  Boost::program_options
  Boost::coroutine
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)
find_package(Boost REQUIRED COMPONENTS program_options coroutine)
add_executable(test_atv320_dbus_iface_integration atv320-dbus-iface-integration.cpp)
target_link_libraries(test_atv320_dbus_iface_integration
  tfc::base
  tfc::ec
  tfc::mock_ipc
  tfc::stub_confman
  tfc::testing
  PkgConfig::SDBUSPLUS
  Boost::program_options
  Boost::coroutine
)

# Get access to private headers
target_include_directories(test_atv320_dbus_iface PRIVATE ${tfc_ec_dirs})

target_include_directories(test_atv320_dbus_iface_integration PRIVATE ${tfc_ec_dirs})

add_test(
  NAME
    test_atv320_dbus_iface
  COMMAND
    test_atv320_dbus_iface
)

add_test(
  NAME
    test_atv320_dbus_iface_integration
  COMMAND
     test_atv320_dbus_iface_integration
)

