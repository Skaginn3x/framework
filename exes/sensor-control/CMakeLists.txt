
find_package(Boost REQUIRED)
#find_package(tfc CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_executable(sensor_control src/main.cpp src/sensor_control.cpp)

target_link_libraries(sensor_control
  PUBLIC
    tfc::base
    tfc::ipc
    tfc::confman
    tfc::logger
    Boost::boost
    fmt::fmt
    glaze::glaze
)

target_include_directories(sensor_control
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${BEXT_SML_INCLUDE_DIRS}
)

add_subdirectory(tests)
