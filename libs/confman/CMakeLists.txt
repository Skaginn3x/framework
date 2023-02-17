project(confman)
cmake_minimum_required(VERSION 3.21)

add_library(confman STATIC
    src/confman.cpp
)
add_library(tfc::confman ALIAS confman)

target_include_directories(confman
    PUBLIC
        inc/public
    PRIVATE
        inc
)

