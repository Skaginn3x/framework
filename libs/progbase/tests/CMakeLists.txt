find_package(ut CONFIG REQUIRED)

add_executable(progbase_options_test options.cpp)
target_link_libraries(progbase_options_test Boost::ut Boost::program_options tfc::base)

add_test(NAME progbase_options_test COMMAND progbase_options_test)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()