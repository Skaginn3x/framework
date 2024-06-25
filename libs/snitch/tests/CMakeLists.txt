find_package(ut CONFIG REQUIRED)

add_executable(snitch_test snitch_test.cpp)
target_link_libraries(snitch_test PRIVATE Boost::ut tfc::snitch tfc::base)
add_test(NAME snitch_test COMMAND snitch_test)
