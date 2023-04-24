project(confman)
cmake_minimum_required(VERSION 3.21)

find_package(glaze CONFIG REQUIRED)
find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")
find_package(fmt CONFIG REQUIRED)

add_library(libconfman
  STATIC
    src/confman.cpp
)
add_library(tfc::confman ALIAS libconfman)

target_include_directories(libconfman
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
    ${AZMQ_INCLUDE_DIRS}
  PRIVATE
    inc
)

target_link_libraries(libconfman
  PUBLIC
    fmt::fmt
    glaze::glaze
    tfc::base
    tfc::rpc
    tfc::ipc
    tfc::stx
)
add_library_to_docs(tfc::confman)

add_subdirectory(tests)
