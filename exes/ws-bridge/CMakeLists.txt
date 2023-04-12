project(tfc-ws-bridge
  VERSION ${CMAKE_PROJECT_VERSION}
  DESCRIPTION
    "A bridge from websockets to zeromq sockets. Linking frontend browser services with internal rpc and ipc mechanisms"
  HOMEPAGE_URL ${CMAKE_PROJECT_HOMEPAGE_URL}
  LANGUAGES CXX
)

add_executable(ws-bridge
  src/ws-bridge.cpp
)

find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(ws-bridge
  PUBLIC
    Boost::boost
    Boost::program_options
    tfc::base
    tfc::confman
    tfc::logger
    tfc::ipc
)

install(
  FILES
    systemd/ws-bridge.service
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/systemd/user/
  CONFIGURATIONS Release
)
install(
  FILES
    systemd/ws-bridge.socket
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/systemd/user/
  CONFIGURATIONS Release
)
install(
  TARGETS
    ws-bridge
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/
  CONFIGURATIONS Release
)
install(
  TARGETS
    ws-bridge
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}/debug/
  CONFIGURATIONS Debug
)
