find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)

add_executable(ipc_manager_test ipc_manager_test.cpp)
add_test(
  NAME
    ipc_manager_test
  COMMAND
    ipc_manager_test
)
target_link_libraries(ipc_manager_test PRIVATE tfc::ipc tfc::confman Boost::ut glaze::glaze)

add_executable(ipc_test ipc_test.cpp)
target_link_libraries(ipc_test PRIVATE Boost::ut tfc::ipc tfc::base)
add_test(NAME ipc_test COMMAND ipc_test)

add_executable(item_test item_test.cpp)
target_link_libraries(item_test PRIVATE Boost::ut tfc::ipc tfc::base)
add_test(NAME item_test COMMAND item_test)

get_property(tfc_ipc_dirs TARGET tfc::ipc PROPERTY INCLUDE_DIRECTORIES)
add_executable(filter_test filter_test.cpp)
target_link_libraries(filter_test PRIVATE Boost::ut tfc::ipc tfc::base tfc::testing tfc::stub_confman)
target_include_directories(filter_test PRIVATE ${tfc_ipc_dirs})
add_test(NAME filter_test COMMAND filter_test)
