project(stx)
cmake_minimum_required(VERSION 3.21)

add_library(stx INTERFACE)
add_library(tfc::stx ALIAS stx)

target_include_directories(stx
  INTERFACE
    inc/public
)
