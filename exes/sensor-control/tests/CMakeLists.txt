
find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_executable(sensor_ctrl_state_machine_tests state_machine_tests.cpp)

target_link_libraries(sensor_ctrl_state_machine_tests
  PRIVATE
    Boost::ut
    tfc::base
    tfc::confman
    tfc::ipc
    GTest::gmock
)

target_include_directories(sensor_ctrl_state_machine_tests
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../inc
    ${BEXT_SML_INCLUDE_DIRS}
)

add_test(
  NAME
    sensor_ctrl_state_machine_tests
  COMMAND
    sensor_ctrl_state_machine_tests
)

