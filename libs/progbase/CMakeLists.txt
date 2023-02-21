project(progbase)
cmake_minimum_required(VERSION 3.21)

add_library(progbase STATIC
    src/progbase.cpp
    )
add_library(tfc::progbase ALIAS progbase)
target_include_directories(progbase
    PUBLIC
    inc/public
    PRIVATE
    inc
    )

find_package(Boost REQUIRED)

target_link_libraries(progbase
    PRIVATE
    Boost::boost
    )
add_library_to_docs(tfc::progbase)
