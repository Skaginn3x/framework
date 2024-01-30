
add_library(mock_motor INTERFACE)
add_library(tfc::mock_motor ALIAS mock_motor)

target_include_directories(mock_motor
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
target_link_libraries(mock_motor
  INTERFACE
    tfc::motor
    GTest::gmock
)

add_library_to_docs(tfc::mock_motor)

add_executable(mock_motor_example example.cpp)
target_link_libraries(mock_motor_example PRIVATE tfc::mock_motor)

include(tfc_install)
tfc_install_lib(mock_motor)
