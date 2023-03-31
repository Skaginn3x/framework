project(confman)

add_executable(confman
    src/main.cpp
)

find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(confman
  PUBLIC
    Boost::boost
    Boost::program_options
    tfc::base
    tfc::confman
    tfc::logger
)