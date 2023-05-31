project(ec)
cmake_minimum_required(VERSION 3.21)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
find_package(fmt CONFIG REQUIRED)
find_package(soem CONFIG REQUIRED)


add_library(ec src/ec.cpp src/base.cpp)
add_library(tfc::ec ALIAS ec)

target_include_directories(ec
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
    ${BEXT_SML_INCLUDE_DIRS}
  PRIVATE
    inc/
)

target_link_libraries(ec
  PUBLIC
    soem
    tfc::ipc
    tfc::confman
  PRIVATE
    fmt::fmt
)

add_library_to_docs(tfc::ec)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()