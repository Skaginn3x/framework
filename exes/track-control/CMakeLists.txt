
find_package(Boost REQUIRED)
#find_package(tfc CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_library(track_ctrl src/track_ctrl.cpp)
add_library(tfc::track_ctrl ALIAS track_ctrl)
add_executable(track_control src/main.cpp)

target_link_libraries(track_ctrl
  PRIVATE
    fmt::fmt
    glaze::glaze
    Boost::boost
)
target_include_directories(track_ctrl
  PUBLIC
    inc/public
  PRIVATE
    inc
    ${BEXT_SML_INCLUDE_DIRS}
)

target_link_libraries(track_control PRIVATE tfc::track_ctrl)
