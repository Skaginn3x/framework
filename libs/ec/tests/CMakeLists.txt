add_executable(test_ec_402 test_ec_402.cpp)
target_link_libraries(test_ec_402 PRIVATE tfc::ec)

add_test(NAME test_ec_402 COMMAND 402_test)

add_subdirectory(examples)