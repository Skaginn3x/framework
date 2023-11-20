
find_package(Boost REQUIRED)
#find_package(tfc CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_library(track_ctrl src/track_ctrl.hpp)
add_library(tfc::track_ctrl ALIAS track_ctrl)
add_executable(track_ctrl src/main.cpp)

target_link_libraries(track_ctrl PRIVATE tfc::track_ctrl)
