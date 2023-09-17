
add_executable(test_beckhoff beckhoff.cpp)
target_link_libraries(test_beckhoff tfc::ec tfc::base)

add_test(
  NAME
    test_beckhoff
  COMMAND
    test_beckhoff
)


