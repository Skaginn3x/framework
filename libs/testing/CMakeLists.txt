project(testing)

add_library(testing
  src/clock.cpp
)
add_library(tfc::testing ALIAS testing)

target_include_directories(testing
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

include(tfc_install)
tfc_install_lib(testing)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

