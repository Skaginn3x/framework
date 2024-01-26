project(ec)
cmake_minimum_required(VERSION 3.21)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
find_package(fmt CONFIG REQUIRED)
find_package(soem CONFIG REQUIRED)
find_package(mp-units CONFIG REQUIRED)

add_library(ec src/ec.cpp src/base.cpp src/devices/beckhoff.cpp src/common.cpp)
add_library(tfc::ec ALIAS ec)

target_include_directories(ec
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
    ${BEXT_SML_INCLUDE_DIRS}
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(ec
  PUBLIC
    soem
    tfc::ipc
    tfc::confman
    tfc::soem_interface
    tfc::stx
    tfc::motor
    mp-units::si
    fmt::fmt
)

add_executable(ethercat src/main.cpp)

find_package(Boost REQUIRED COMPONENTS program_options coroutine)
target_link_libraries(ethercat
  PUBLIC
    tfc::ec
    Boost::program_options
    Boost::coroutine
)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(GNUInstallDirs)

configure_file("${CMAKE_CURRENT_LIST_DIR}/systemd/ethercat.service" "${CMAKE_BINARY_DIR}/systemd/ethercat.service")

install(
  FILES "${CMAKE_BINARY_DIR}/systemd/ethercat.service"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/systemd/system/"
  CONFIGURATIONS Release
)

install(
  TARGETS
    ethercat
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    ethercat
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
