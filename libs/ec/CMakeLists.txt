project(ec)
cmake_minimum_required(VERSION 3.21)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
find_path(ETHERCAT_INCLUDE_DIRS "ethercat/ecrt.h")
find_path(SOEM_INCLUDE_DIRS "soem/ethercat.h")
find_package(fmt CONFIG REQUIRED)
find_package(mp-units CONFIG REQUIRED)


add_library(ec STATIC src/ec.cpp src/base.cpp)
add_library(tfc::ec ALIAS ec)

target_include_directories(ec
  PUBLIC
    inc/public
  ${BEXT_SML_INCLUDE_DIRS}
  ${SOEM_INCLUDE_DIRS}/soem
  PRIVATE
    inc/
)
target_link_directories(ec PUBLIC ${SOEM_INCLUDE_DIRS}/../lib/)

target_link_libraries(ec
  PUBLIC
    libsoem.a
    tfc::ipc
  PRIVATE
    fmt::fmt
    mp-units::si
    mp-units::isq
    mp-units::core
    mp-units::si-cgs
  )

add_library_to_docs(tfc::ec)

add_subdirectory(tests)