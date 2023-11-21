
add_executable(and and.cpp)

find_package(fmt CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)

target_link_libraries(and
  PUBLIC
    tfc::base
    tfc::ipc
    tfc::confman
    tfc::logger
    tfc::operation_mode
    fmt::fmt
    glaze::glaze
)

