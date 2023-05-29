add_executable(tfctui
  src/button.cpp
  src/expander.cpp
  src/keybinding.cpp
  src/main.cpp
  src/main_ui.cpp
  src/mytoggle.cpp
)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(ftxui CONFIG REQUIRED)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/version.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/inc/version.hpp
)

target_link_libraries(tfctui
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    ftxui::dom ftxui::screen ftxui::component
    Boost::program_options
)

target_include_directories(tfctui
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/inc
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
