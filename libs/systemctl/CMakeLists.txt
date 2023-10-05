function(tfc_systemd_service_file EXE_TARGET DESCRIPTION)

set(EXE_NAME ${EXE_TARGET})
set(INSTALL_DIR ${CMAKE_INSTALL_BINDIR})

configure_file("${CMAKE_SOURCE_DIR}/libs/systemctl/systemd/template.service.in" "${CMAKE_BINARY_DIR}/systemd/${EXE_TARGET}.service")

install(
  FILES "${CMAKE_BINARY_DIR}/systemd/${EXE_TARGET}.service"
  DESTINATION /usr/lib/systemd/system/
  CONFIGURATIONS Release
)

endfunction()
