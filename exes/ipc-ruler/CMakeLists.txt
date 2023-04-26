add_executable(ipc-ruler src/ipc-ruler.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(sdbus-c++ CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(PKG_SDBUSPLUS REQUIRED IMPORTED_TARGET sdbusplus)

target_link_libraries(ipc-ruler
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    Boost::program_options
    SDBusCpp::sdbus-c++
    PkgConfig::PKG_SDBUSPLUS
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
