add_executable(ttui src/ttui.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(mp-units CONFIG REQUIRED)
find_package(ftxui CONFIG REQUIRED)

target_link_libraries(ttui
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    Boost::program_options
    mp-units::core
    ftxui::dom
    ftxui::screen
    ftxui::component
)

include(GNUInstallDirs)
install(
  TARGETS
    ttui
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    ttui
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
