
find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_executable(sensor_ctrl_state_machine_test state_machine_test.cpp)

target_link_libraries(sensor_ctrl_state_machine_test
  PRIVATE
    Boost::ut
    GTest::gmock
    tfc::base
    tfc::stx
    tfc::ipc
#    tfc::sensor_ctrl
)
get_property(tfc_sensor_ctrl_dirs TARGET tfc::sensor_ctrl PROPERTY INCLUDE_DIRECTORIES)
message(WARNING "tfc_sensor_ctrl_dirs: ${tfc_sensor_ctrl_dirs}")
target_link_directories(sensor_ctrl_state_machine_test
  PRIVATE
    ${tfc_sensor_ctrl_dirs}
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

