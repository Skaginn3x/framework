project(confman)
cmake_minimum_required(VERSION 3.21)

add_library(libconfman STATIC
    src/confman.cpp
)
add_library(tfc::confman ALIAS libconfman)

target_include_directories(libconfman
    PUBLIC
        inc/public
    PRIVATE
        inc
)

add_library_to_docs(tfc::confman)
