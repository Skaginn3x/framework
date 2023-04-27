
find_package(ut CONFIG REQUIRED)

add_executable(liboperation_mode_test liboperation_mode_test.cpp)

target_link_libraries(liboperation_mode_test PRIVATE Boost::ut tfc::logger tfc::base tfc::operation_mode)

add_test(
  NAME
    liboperation_mode_test
  COMMAND
    liboperation_mode_test
)
