
add_executable(op_enable main.cpp)

find_package(fmt CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)

target_link_libraries(op_enable
  PUBLIC
    tfc::base
    tfc::ipc
    tfc::confman
    tfc::logger
    tfc::operation_mode
    fmt::fmt
    glaze::glaze
)
