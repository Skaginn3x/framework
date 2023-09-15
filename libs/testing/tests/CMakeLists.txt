find_package(ut CONFIG REQUIRED)

add_executable(clock_test clock_test.cpp)
target_link_libraries(clock_test Boost::ut tfc::base tfc::testing)

add_test(NAME clock_test COMMAND clock_test)
