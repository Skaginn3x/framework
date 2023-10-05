function(tfc_systemd_service_file EXE_TARGET DESCRIPTION)

set(EXE_NAME ${EXE_TARGET})
set(INSTALL_DIR ${CMAKE_INSTALL_BINDIR})

configure_file("${CMAKE_SOURCE_DIR}/cmake/systemd/tfc@.service" "${CMAKE_BINARY_DIR}/systemd/tfc@${EXE_TARGET}.service")

install(
  FILES "${CMAKE_BINARY_DIR}/systemd/tfc@${EXE_TARGET}.service"
  DESTINATION /usr/lib/systemd/system/
  CONFIGURATIONS Release
)

endfunction()

