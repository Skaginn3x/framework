project(confman)
cmake_minimum_required(VERSION 3.21)

find_package(glaze CONFIG REQUIRED)
find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")

add_library(libconfman
  STATIC
    src/confman.cpp
)
add_library(tfc::confman ALIAS libconfman)

target_include_directories(libconfman
  PUBLIC
    inc/public
    ${AZMQ_INCLUDE_DIRS}
  PRIVATE
    inc
)

target_link_libraries(libconfman
  PRIVATE
    glaze::glaze
    tfc::rpc
)
add_library_to_docs(tfc::confman)

add_subdirectory(tests)
