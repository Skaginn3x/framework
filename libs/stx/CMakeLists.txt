project(stx)
cmake_minimum_required(VERSION 3.21)

add_library(stx INTERFACE)
add_library(tfc::stx ALIAS stx)

target_include_directories(stx
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
)

# Those links should be minimized, consider splitting utils and stx if utils needs further dependencies
target_link_libraries(stx
  INTERFACE
    tfc::configure_options
)
