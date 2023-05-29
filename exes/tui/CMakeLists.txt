add_executable(tfctui tui.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(ftxui CONFIG REQUIRED)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version.hpp
)

target_link_libraries(tfctui
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    ftxui::dom ftxui::screen ftxui::component
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
