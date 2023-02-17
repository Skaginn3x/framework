project(ipc)
cmake_minimum_required(VERSION 3.21)

add_library(ipc STATIC
    src/ipc.cpp
)
add_library(tfc::ipc ALIAS ipc)
target_include_directories(ipc
    PUBLIC
        inc/public
    PRIVATE
        inc
)

