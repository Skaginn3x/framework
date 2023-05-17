project(operation_mode)
cmake_minimum_required(VERSION 3.21)

add_executable(operation-mode
  src/main.cpp
  src/app_operation_mode.cpp
  src/state_machine.cpp
)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
target_include_directories(operation-mode
  PUBLIC
    inc
    ${BEXT_SML_INCLUDE_DIRS}
)

find_package(Boost REQUIRED)
find_package(unofficial-sdbusplus CONFIG REQUIRED)
target_link_libraries(operation-mode
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    tfc::ipc
    tfc::operation_mode
    tfc::confman
    Boost::boost
    unofficial::sdbusplus
)

add_subdirectory(tests)

