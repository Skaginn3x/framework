find_package(ut CONFIG REQUIRED)

add_executable(motor_impl_test motor_impl_test.cxx)

target_link_libraries(motor_impl_test PRIVATE Boost::ut tfc::motor tfc::base)

add_test(
    NAME
    motor_impl_test
    COMMAND
    motor_impl_test
)
add_subdirectory(example)