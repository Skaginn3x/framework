add_subdirectory(stx)
add_subdirectory(progbase)
add_subdirectory(logger)
add_subdirectory(confman)
add_subdirectory(ipc)
add_subdirectory(rpc)
add_subdirectory(ec)

add_library(tfc STATIC)
add_library(tfc::tfc ALIAS tfc)
target_link_libraries(tfc
  PUBLIC
    tfc::logger,
    tfc::confman,
    tfc::base,
    tfc::ipc,
    tfc::rpc,
    tfc::ec
)
