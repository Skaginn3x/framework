add_subdirectory(configure_options)
add_subdirectory(dbus_util)
add_subdirectory(stx)
add_subdirectory(logger)
add_subdirectory(progbase)
add_subdirectory(confman)
add_subdirectory(ipc)
add_subdirectory(rpc)
add_subdirectory(ec)
add_subdirectory(operation_mode)

add_library(tfc INTERFACE)
add_library(tfc::tfc ALIAS tfc)
target_link_libraries(tfc
  INTERFACE
    tfc::stx
    tfc::base
    tfc::logger
    tfc::confman
    tfc::ipc
)
