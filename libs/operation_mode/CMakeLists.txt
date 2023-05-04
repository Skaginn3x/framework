project(operation_mode)
cmake_minimum_required(VERSION 3.21)

add_library(liboperation_mode
  src/operation_mode.cpp
)
add_library(tfc::operation_mode ALIAS liboperation_mode)

target_include_directories(liboperation_mode
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
  PRIVATE
    inc
)
set_target_properties(liboperation_mode PROPERTIES PREFIX "")

find_package(Boost REQUIRED)
find_package(unofficial-sdbusplus CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(liboperation_mode
  PUBLIC
    tfc::base
    tfc::logger
    tfc::stx
    glaze::glaze
    magic_enum::magic_enum
  PRIVATE
    unofficial::sdbusplus
    Boost::boost
)

add_library_to_docs(tfc::operation_mode)

add_subdirectory(tests)
