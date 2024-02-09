add_executable(button src/main.cpp)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
target_include_directories(button PRIVATE ${BEXT_SML_INCLUDE_DIRS})

find_package(glaze CONFIG REQUIRED)

target_link_libraries(button
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
)

include(tfc_split_debug_info)
tfc_split_debug_info(button)

include(GNUInstallDirs)
install(
  TARGETS
    button
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    button
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
