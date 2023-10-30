
find_package(Boost REQUIRED)
#find_package(tfc CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_library(sensor_ctrl src/sensor_control.cpp)
add_library(tfc::sensor_ctrl ALIAS sensor_ctrl)
add_executable(sensor_control src/main.cpp)

target_link_libraries(sensor_ctrl
  PUBLIC
    tfc::base
    tfc::ipc
    tfc::confman
    tfc::logger
    tfc::operation_mode
    Boost::boost
    fmt::fmt
    glaze::glaze
)
target_link_libraries(sensor_control
  PUBLIC
    tfc::sensor_ctrl
)

target_include_directories(sensor_ctrl
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${BEXT_SML_INCLUDE_DIRS}
)

add_subdirectory(tests)
