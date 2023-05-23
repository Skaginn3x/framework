project(mock)
cmake_minimum_required(VERSION 3.21)

add_library(mock INTERFACE)
add_library(tfc::mock ALIAS mock)

target_include_directories(mock
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
)

#target_link_libraries(mock
#  PUBLIC
#)

add_library_to_docs(tfc::mock)

add_subdirectory(example)
