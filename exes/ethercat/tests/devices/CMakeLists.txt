

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
target_link_libraries(test_atv320_dbus_iface tfc::base tfc::ec tfc::mock_ipc tfc::stub_confman)

# Get access to private headers
get_property(tfc_ec_dirs TARGET tfc::ec PROPERTY INCLUDE_DIRECTORIES)
target_include_directories(test_atv320_dbus_iface PRIVATE ${tfc_ec_dirs})

add_test(
  NAME
    test_atv320_dbus_iface
  COMMAND
    test_atv320_dbus_iface
)

