project(progbase)
cmake_minimum_required(VERSION 3.21)

add_library(base
  src/progbase.cpp
)
add_library(tfc::base ALIAS base)
get_target_property(TFC_LOGGER_INCLUDE_DIRS tfc::logger INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(base
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
  PRIVATE
    inc
    ${TFC_LOGGER_INCLUDE_DIRS}
)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(base
  PRIVATE
    Boost::boost
    Boost::program_options
    fmt::fmt
    tfc::stx
)

add_library_to_docs(tfc::base)

add_subdirectory(tests)
