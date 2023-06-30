
add_executable(confman-example-simple simple.cpp)

find_package(mp-units CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(confman-example-simple PRIVATE tfc::confman mp-units::si fmt::fmt)

add_executable(confman-example-nested nested.cpp)
target_link_libraries(confman-example-nested PRIVATE tfc::confman mp-units::si fmt::fmt)

add_executable(confman-example-array array.cpp)
target_link_libraries(confman-example-array PRIVATE tfc::confman mp-units::si fmt::fmt)

add_executable(confman-example-variant variant.cpp)
target_link_libraries(confman-example-variant PRIVATE tfc::confman mp-units::si fmt::fmt)
