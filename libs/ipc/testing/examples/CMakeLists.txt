tfc_add_example(ipc_connector_example ipc_connector_example.cpp)
target_link_libraries(ipc_connector_example PRIVATE tfc::ipc)

tfc_add_example(print_signals_and_slots print_signals_and_slots.cpp)
target_link_libraries(print_signals_and_slots PRIVATE tfc::base tfc::ipc)

tfc_add_example_no_test(ipc_coroutines ipc_coroutines.cpp)
target_link_libraries(ipc_coroutines PRIVATE tfc::base tfc::ipc)

find_package(mp-units CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
tfc_add_example_no_test(mass_example mass_example.cpp)
target_link_libraries(mass_example PRIVATE tfc::base tfc::ipc mp-units::si fmt::fmt)
