project(button)
add_executable(${PROJECT_NAME} src/main.cpp)

find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")
target_include_directories(${PROJECT_NAME} PRIVATE ${BEXT_SML_INCLUDE_DIRS})

find_package(glaze CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
)

include(tfc_split_debug_info)
tfc_split_debug_info(${PROJECT_NAME})

include(GNUInstallDirs)
install(
  TARGETS
    ${PROJECT_NAME}
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    ${PROJECT_NAME}
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)

include(tfc_systemd)
tfc_systemd_service_file(${PROJECT_NAME} "TFC Button, make use of short/long press etc.")
