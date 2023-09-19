add_executable(test_ec_402 test_ec_402.cpp)
target_link_libraries(test_ec_402 PRIVATE tfc::ec)

add_executable(test_ec_util test_ec_util.cpp)
target_link_libraries(test_ec_util tfc::ec)

add_test(
  NAME
    test_ec_402
  COMMAND
    test_ec_402
)
add_test(
  NAME
    test_ec_util
  COMMAND
    test_ec_util
)

add_subdirectory(devices)
