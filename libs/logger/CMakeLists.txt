project(logger)
cmake_minimum_required(3.21)

add_library(logger STATIC
    src/logger.cpp
)
add_library(tfc::logger ALIAS logger)
target_include_directories(logger
    PUBLIC
        inc/public
    PRIVATE
        inc
)

