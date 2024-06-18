project(snitch)

add_library(snitch
    src/snitch.cpp
)
add_library(tfc::snitch ALIAS snitch)
target_include_directories(snitch
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)

find_package(Boost REQUIRED COMPONENTS )
find_package(fmt CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)

target_link_libraries(snitch
  PUBLIC
    fmt::fmt
    Boost::boost
    tfc::stx
    tfc::logger
    tfc::dbus_util
  PRIVATE
    tfc::base
    PkgConfig::SDBUSPLUS
)

add_library_to_docs(tfc::snitch)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(tfc_install)
tfc_install_lib(snitch)
