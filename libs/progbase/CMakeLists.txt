project(progbase)
cmake_minimum_required(VERSION 3.28)

add_library(base)
target_sources(base
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        inc/public/tfc/progbase.cxx
)
add_library(tfc::base ALIAS base)

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

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

#include(tfc_install)
#tfc_install_lib(base)
