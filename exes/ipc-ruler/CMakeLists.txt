add_executable(ipc-ruler src/ipc-ruler.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)

find_package(unofficial-sdbusplus CONFIG REQUIRED)

target_link_libraries(ipc-ruler
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    tfc::dbus_util
    tfc::ipc_connector
    Boost::program_options
    unofficial::sdbusplus
)

include(GNUInstallDirs)
install(
  TARGETS
    ipc-ruler
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    ipc-ruler
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
