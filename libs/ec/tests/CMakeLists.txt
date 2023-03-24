add_executable(test_ec_402 test_ec_402.cpp)
target_link_libraries(test_ec_402 PRIVATE tfc::ec)

add_test(NAME test_ec_402 COMMAND test_ec_402)

add_executable(run_motor_soem run_motor_soem.cpp)
target_link_libraries(run_motor_soem tfc::ec)

add_executable(run_motor_soem_cppversion run_motor_soem_cppversion.cpp)
target_link_libraries(run_motor_soem_cppversion tfc::ec)

add_subdirectory(examples)