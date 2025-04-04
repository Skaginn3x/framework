
find_package(ut CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_executable(operation_mode_integration_test operation_mode_integration_test.cpp)
add_executable(operations_state_machine_test operations_state_machine_test.cpp)
add_executable(operations_state_machine_owner_test operations_state_machine_owner_test.cpp)

target_link_libraries(operation_mode_integration_test
  PRIVATE
    Boost::ut
    tfc::base
    tfc::confman
    tfc::ipc
    tfc::operation_mode
    tfc::mock_ipc
)
target_link_libraries(operations_state_machine_test
  PRIVATE
    tfc::base
    tfc::operation_mode
    GTest::gmock
    Boost::ut
)
target_link_libraries(operations_state_machine_owner_test
    PRIVATE
    tfc::base
    tfc::operation_mode
    tfc::mock_ipc
    GTest::gmock
    Boost::ut
)
target_include_directories(operations_state_machine_test
  PRIVATE
    ${BEXT_SML_INCLUDE_DIRS}
)
target_include_directories(operations_state_machine_owner_test
  PRIVATE
    ${BEXT_SML_INCLUDE_DIRS}
)

target_include_directories(operation_mode_integration_test
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../inc
)
target_include_directories(operations_state_machine_test
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../inc
)
target_include_directories(operations_state_machine_owner_test
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../inc
)

target_sources(operation_mode_integration_test
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/app_operation_mode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/state_machine_owner.cpp
)

add_test(NAME operation_mode_integration_test COMMAND operation_mode_integration_test)
add_test(NAME operations_state_machine_test COMMAND operations_state_machine_test)
add_test(NAME operations_state_machine_owner_test COMMAND operations_state_machine_owner_test)

