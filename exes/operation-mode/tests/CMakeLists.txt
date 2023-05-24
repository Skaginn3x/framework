
find_package(ut CONFIG REQUIRED)

add_executable(operation_mode_integration_test operation_mode_integration_test.cpp)

target_link_libraries(operation_mode_integration_test
  PRIVATE
    Boost::ut
    tfc::base
    tfc::confman
    tfc::ipc
    tfc::operation_mode
)

target_include_directories(operation_mode_integration_test
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../inc
)

target_sources(operation_mode_integration_test
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/app_operation_mode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/state_machine.cpp
)

add_test(
  NAME
    operation_mode_integration_test
  COMMAND
    operation_mode_integration_test
)

