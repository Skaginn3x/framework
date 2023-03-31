project(progbase)
cmake_minimum_required(VERSION 3.21)

add_library(base
  STATIC
    src/progbase.cpp
)
add_library(tfc::base ALIAS base)
target_include_directories(base
  PUBLIC
    inc/public
  PRIVATE
    inc
)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(base
  PRIVATE
    Boost::boost
    Boost::program_options
    fmt::fmt
    tfc::stx
    tfc::logger
)

add_library_to_docs(tfc::base)

add_subdirectory(tests)
