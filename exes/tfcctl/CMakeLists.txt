add_executable(tfcctl src/tfcctl.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(tfcctl
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    Boost::program_options
)

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
