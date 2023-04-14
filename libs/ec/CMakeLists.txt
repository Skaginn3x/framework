project(ec)
cmake_minimum_required(VERSION 3.21)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
find_package(fmt CONFIG REQUIRED)
find_package(mp-units CONFIG REQUIRED)
find_package(soem CONFIG REQUIRED)


add_library(ec STATIC src/ec.cpp src/base.cpp)
add_library(tfc::ec ALIAS ec)

target_include_directories(ec
  PUBLIC
    inc/public
    ${BEXT_SML_INCLUDE_DIRS}
  PRIVATE
    inc/
)

target_link_libraries(ec
  PUBLIC
    soem
    tfc::ipc
    tfc::ipc_connector
  PRIVATE
    fmt::fmt
    mp-units::si
    mp-units::isq
    mp-units::core
    mp-units::si-cgs
)

add_library_to_docs(tfc::ec)

add_subdirectory(tests)