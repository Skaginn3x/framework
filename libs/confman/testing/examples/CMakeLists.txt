find_package(mp-units CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(confman_example_simple simple.cpp)
target_link_libraries(confman_example_simple PRIVATE tfc::confman mp-units::si fmt::fmt)

add_executable(confman_example_nested nested.cpp)
target_link_libraries(confman_example_nested PRIVATE tfc::confman mp-units::si fmt::fmt)

add_executable(confman_example_array array.cpp)
target_link_libraries(confman_example_array PRIVATE tfc::confman mp-units::si fmt::fmt)

add_executable(confman_example_variant variant.cpp)
target_link_libraries(confman_example_variant PRIVATE tfc::confman mp-units::si fmt::fmt)

add_executable(confman_example_item confman_example_item.cpp)
target_link_libraries(confman_example_item PRIVATE tfc::confman tfc::ipc mp-units::si fmt::fmt)

add_executable(confman_example_useless useless-box.cpp)
target_link_libraries(confman_example_useless PRIVATE tfc::confman tfc::ipc mp-units::si fmt::fmt)
