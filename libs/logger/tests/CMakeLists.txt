
find_package(ut CONFIG REQUIRED)

add_executable(logging_test logging_test.cpp)

target_link_libraries(logging_test PRIVATE Boost::ut tfc::logger tfc::base)

add_test(
  NAME
    logging_test
  COMMAND
    logging_test
)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()