find_package(ut CONFIG REQUIRED)
add_executable(on_change_test on_change_test.cpp)

target_link_libraries(on_change_test PRIVATE tfc::confman Boost::ut glaze::glaze)

add_executable(azmq_server_testing azmq_server_testing.cpp)

target_include_directories(azmq_server_testing PRIVATE ${AZMQ_INCLUDE_DIRS})

add_test(
  NAME
  on_change_test
  COMMAND
  on_change_test
)

add_subdirectory(examples)