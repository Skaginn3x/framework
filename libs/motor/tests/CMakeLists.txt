find_package(ut CONFIG REQUIRED)

add_executable(motor_impl_test motor_impl_test.cxx)

target_link_libraries(motor_impl_test PRIVATE Boost::ut tfc::motor tfc::base)

add_test(
  NAME
    motor_impl_test
  COMMAND
    motor_impl_test
)

add_executable(positioner_test positioner_test.cpp)
target_link_libraries(positioner_test PRIVATE Boost::ut tfc::motor tfc::base tfc::mock_ipc tfc::testing)
add_test(
  NAME
    positioner_test
  COMMAND
    positioner_test
)
# todo remove when sdbusplus starts behaving
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(positioner_test PRIVATE DEBUG)
endif()
if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()