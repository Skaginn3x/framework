add_executable(button src/main.cpp)

find_path(AZMQ_INCLUDE_DIRS "azmq/actor.hpp")

target_include_directories(button
  PUBLIC
    ${AZMQ_INCLUDE_DIRS}
)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(glaze CONFIG REQUIRED)

target_link_libraries(button
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    tfc::confman
    Boost::program_options
    glaze::glaze
)

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
