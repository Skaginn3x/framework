
add_executable(confman-simple simple.cpp)

find_package(mp-units CONFIG REQUIRED)
target_link_libraries(confman-simple PRIVATE tfc::confman mp-units::si)
