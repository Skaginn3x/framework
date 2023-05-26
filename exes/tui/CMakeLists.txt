add_executable(tfctui tui.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(tfctui
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    Boost::program_options
)

include(GNUInstallDirs)
install(
  TARGETS
    tfctui
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    tfctui
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
