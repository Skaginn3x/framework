add_executable(tfcctl src/tfcctl.cpp)


target_link_libraries(tfcctl
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
)

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
