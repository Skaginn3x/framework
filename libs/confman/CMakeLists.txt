project(confman)

find_package(glaze CONFIG REQUIRED)
find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS )
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUS REQUIRED IMPORTED_TARGET GLOBAL sdbusplus)

add_library(confman
  src/confman.cpp
  src/remote_change.cpp
  src/file_storage.cpp
  src/detail/config_dbus_client.cpp
)
add_library(tfc::confman ALIAS confman)

target_include_directories(confman
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${AZMQ_INCLUDE_DIRS}>
)

target_link_libraries(confman
  PUBLIC
    Boost::boost
    fmt::fmt
    glaze::glaze
    tfc::base
    tfc::stx
    tfc::dbus_util
    tfc::logger
  PRIVATE
    PkgConfig::SDBUSPLUS
)
add_library_to_docs(tfc::confman)

add_subdirectory(testing)

include(tfc_install)
tfc_install_lib(confman)
