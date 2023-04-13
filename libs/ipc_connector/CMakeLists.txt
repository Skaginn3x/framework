project(ipc)
cmake_minimum_required(VERSION 3.21)

add_library(ipc_connector
  src/ipc_connector.cpp
)
add_library(tfc::ipc_connector ALIAS ipc_connector)

target_include_directories(ipc_connector
  PUBLIC
    inc/public
  PRIVATE
    inc
)

target_link_libraries(ipc_connector
  PUBLIC
    tfc::ipc
    tfc::confman
)

add_library_to_docs(tfc::ipc_connector)

add_subdirectory(tests)
