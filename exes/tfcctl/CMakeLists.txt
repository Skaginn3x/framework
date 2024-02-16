add_executable(tfcctl src/tfcctl.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(mp-units CONFIG REQUIRED)

target_link_libraries(tfcctl
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    Boost::program_options
    mp-units::core
)

include(tfc_split_debug_info)
tfc_split_debug_info(tfcctl)

include(GNUInstallDirs)
install(
  TARGETS
    tfcctl
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    tfcctl
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
