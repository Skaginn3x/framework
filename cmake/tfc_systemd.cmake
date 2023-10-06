function(tfc_systemd_service_file EXE_TARGET DESCRIPTION SERVICE_USER)

  if (NOT SERVICE_USER)
    set(SERVICE_USER "")
  endif ()

  set(EXE_NAME ${EXE_TARGET})
  set(INSTALL_DIR ${CMAKE_INSTALL_BINDIR})
  set(USER ${SERVICE_USER})

  configure_file("${CMAKE_SOURCE_DIR}/cmake/systemd/tfc@.service" "${CMAKE_BINARY_DIR}/systemd/${EXE_TARGET}@.service")

  install(
    FILES "${CMAKE_BINARY_DIR}/systemd/${EXE_TARGET}@.service"
    DESTINATION /usr/lib/systemd/system/
    CONFIGURATIONS Release
  )

endfunction()
