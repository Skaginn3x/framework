

add_executable(test_beckhoff beckhoff.cpp)
target_link_libraries(test_beckhoff tfc::base tfc::ec)

# Get access to private headers
get_property(tfc_ec_dirs TARGET tfc::ec PROPERTY INCLUDE_DIRECTORIES)
target_include_directories(test_beckhoff PRIVATE ${tfc_ec_dirs})

add_test(
  NAME
    test_beckhoff
  COMMAND
    test_beckhoff
)
