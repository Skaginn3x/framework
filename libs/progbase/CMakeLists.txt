project(progbase)

add_library(base
  src/progbase.cpp
)
add_library(tfc::base ALIAS base)
get_target_property(TFC_LOGGER_INCLUDE_DIRS tfc::logger INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(base
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    inc
    ${CMAKE_CURRENT_BINARY_DIR}/inc
    ${TFC_LOGGER_INCLUDE_DIRS}
)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(fmt CONFIG REQUIRED)


target_link_libraries(base
  PRIVATE
    Boost::boost
    Boost::program_options
    fmt::fmt
    tfc::stx
)

add_library_to_docs(tfc::base)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/inc/tfc/version.hpp
  ALL
  COMMAND
    ${CMAKE_COMMAND} -D NAMESPACE=tfc -D OUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/inc/tfc/version.hpp -P ${CMAKE_SOURCE_DIR}/cmake/tfc_version.cmake
  WORKING_DIRECTORY
    ${CMAKE_SOURCE_DIR}
)

add_custom_target(generate_version_header
  ALL
  COMMAND
    ${CMAKE_COMMAND} -D NAMESPACE=tfc -D OUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/inc/tfc/version.hpp -P ${CMAKE_SOURCE_DIR}/cmake/tfc_version.cmake
  WORKING_DIRECTORY
    ${CMAKE_SOURCE_DIR}
)
add_dependencies(base generate_version_header)
include(tfc_install)
tfc_install_lib(base)
