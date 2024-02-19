
add_library(mock_confman INTERFACE)
add_library(tfc::mock_confman ALIAS mock_confman)

target_include_directories(mock_confman
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
target_link_libraries(mock_confman
  INTERFACE
    tfc::confman
    GTest::gmock
)

add_library_to_docs(tfc::mock_confman)

include(tfc_install)
tfc_install_lib(mock_confman)

if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()