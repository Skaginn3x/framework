project(ipc)

add_library(ipc
  src/ipc.cpp
  src/dbus_client_iface_mock.cpp # todo make ipc_test target
  src/dbus_client_iface.cpp
  src/item.cpp
)
add_library(tfc::ipc ALIAS ipc)

find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")

target_include_directories(ipc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${AZMQ_INCLUDE_DIRS}>
)

find_package(Boost REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_path(PCG_INCLUDE_DIRS "pcg_random.hpp")

target_link_libraries(ipc
  PUBLIC
    tfc::stx
    tfc::logger
    tfc::base
    tfc::dbus_util
    tfc::confman
    Boost::boost
    fmt::fmt
    glaze::glaze
    stduuid
    $<IF:$<BOOL:${ZeroMQ_LIBRARY}>,${ZeroMQ_LIBRARY},${ZeroMQ_STATIC_LIBRARY}>
)

target_include_directories(ipc PRIVATE ${PCG_INCLUDE_DIRS})

add_library_to_docs(tfc::ipc)

add_subdirectory(testing)

include(tfc_install)
tfc_install_lib(ipc)
