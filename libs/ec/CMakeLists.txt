project(ec)
cmake_minimum_required(VERSION 3.21)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_library(ec STATIC src/ec.cpp)
add_library(tfc::ec ALIAS ec)

target_include_directories(ec
  PUBLIC
    inc/public
  ${BEXT_SML_INCLUDE_DIRS}
)

#add_library_to_docs(tfc::ec)

add_subdirectory(tests)