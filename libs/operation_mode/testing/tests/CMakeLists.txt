find_package(ut CONFIG REQUIRED)

add_executable(mock_operation_test mock_operation_test.cpp)

target_link_libraries(mock_operation_test PRIVATE Boost::ut Boost::boost tfc::base tfc::mock_operation)

add_test(
  NAME
    mock_operation_test
  COMMAND
    mock_operation_test
)
