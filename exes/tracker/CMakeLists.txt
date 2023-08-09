add_executable(tracker
  src/conveyor.cpp
  src/entry.cpp
  src/exit.cpp
  src/main.cpp
  src/system.cpp
)

find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(tracker
  PUBLIC
    tfc::ipc
    tfc::base
    tfc::logger
    tfc::confman
    Boost::program_options
)

target_include_directories(tracker
  PUBLIC
    inc
)

include(GNUInstallDirs)
install(
  TARGETS
    tracker
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  CONFIGURATIONS Release
)

install(
  TARGETS
    tracker
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
