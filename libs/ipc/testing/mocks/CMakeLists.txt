
add_library(mock_ipc INTERFACE)
add_library(tfc::mock_ipc ALIAS mock_ipc)

target_include_directories(mock_ipc
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
target_link_libraries(mock_ipc
  INTERFACE
    tfc::ipc
    GTest::gmock
)

add_library_to_docs(tfc::mock_ipc)

include(tfc_install)
tfc_install_lib(mock_ipc)
