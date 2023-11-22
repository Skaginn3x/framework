find_package(ut CONFIG REQUIRED)

add_executable(motor_impl_test motor_impl_test.cxx)
add_executable(sdbusplus_ping_test sdbusplus_ping_test.cpp)

target_link_libraries(motor_impl_test PRIVATE Boost::ut tfc::motor tfc::base)

find_package(Boost REQUIRED COMPONENTS )
target_link_libraries(sdbusplus_ping_test PRIVATE PkgConfig::SDBUSPLUS Boost::boost)

add_test(
    NAME
    motor_impl_test
    COMMAND
    motor_impl_test
)
if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()