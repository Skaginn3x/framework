
add_library(mock_operation
    src/operation_mode.cpp
)
add_library(tfc::mock_operation ALIAS mock_operation)

target_include_directories(mock_operation
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
target_link_libraries(mock_operation
  INTERFACE
    tfc::operation_mode
    GTest::gmock
)

add_library_to_docs(tfc::mock_operation)

include(tfc_install)
tfc_install_lib(mock_operation)
