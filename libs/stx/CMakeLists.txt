project(stx)
cmake_minimum_required(VERSION 3.21)

add_library(stx INTERFACE)
add_library(tfc::stx ALIAS stx)

target_include_directories(stx
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
)

find_package(date CONFIG REQUIRED)
find_package(mp-units CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)

target_link_libraries(stx
  INTERFACE
    date::date
    glaze::glaze
    mp-units::si
    mp-units::core
    stduuid
)

include(tfc_install)
tfc_install_lib(stx)

add_subdirectory(tests)
