
find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_executable(sensor_ctrl_state_machine_test state_machine_test.cpp)

target_link_libraries(sensor_ctrl_state_machine_test
  PRIVATE
    Boost::ut
    GTest::gmock
    tfc::sensor_ctrl
)

add_test(
  NAME
    sensor_ctrl_state_machine_test
  COMMAND
    sensor_ctrl_state_machine_test
)

add_executable(sensor_control_test sensor_control_test.cpp)
target_link_libraries(sensor_control_test
  PRIVATE
    Boost::ut
    tfc::sensor_ctrl
    tfc::mock_ipc
)
add_test(
  NAME
    sensor_control_test
  COMMAND
    sensor_control_test
)

