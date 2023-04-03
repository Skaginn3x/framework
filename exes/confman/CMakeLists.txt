project(tfc-confman
  VERSION ${CMAKE_PROJECT_VERSION}
  DESCRIPTION
    "Configuration manager daemon, communicates via unix file sockets providing RPC and notification layer."
  HOMEPAGE_URL ${CMAKE_PROJECT_HOMEPAGE_URL}
  LANGUAGES CXX
)

add_executable(confman
    src/main.cpp
)

find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(confman
  PUBLIC
    Boost::boost
    Boost::program_options
    tfc::base
    tfc::confman
    tfc::logger
)

install(
  FILES
    systemd/confman.service
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/systemd/user/
  CONFIGURATIONS Release
)
install(
  FILES
    systemd/confman.socket
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/systemd/user/
  CONFIGURATIONS Release
)
install(
  TARGETS
    confman
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/
  CONFIGURATIONS Release
)
install(
  TARGETS
    confman
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
