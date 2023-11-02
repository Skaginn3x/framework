project(testing)

add_library(testing
  src/asio_clock.cpp
)
add_library(tfc::testing ALIAS testing)

target_include_directories(testing
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

include(tfc_install)
tfc_install_lib(testing)

add_subdirectory(tests)
