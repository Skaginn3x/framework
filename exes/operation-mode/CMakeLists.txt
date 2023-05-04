project(operation_mode)
cmake_minimum_required(VERSION 3.21)

add_executable(operation-mode
  src/main.cpp
  src/app_operation_mode.cpp
)

target_include_directories(operation-mode
  PUBLIC
    inc
)

find_package(Boost REQUIRED)
find_package(unofficial-sdbusplus CONFIG REQUIRED)
target_link_libraries(operation-mode
  PUBLIC
    tfc::base
    tfc::logger
    tfc::operation_mode
    Boost::boost
    unofficial::sdbusplus
)



