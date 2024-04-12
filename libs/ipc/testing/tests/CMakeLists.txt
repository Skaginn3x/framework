find_package(ut CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
add_executable(ipc_manager_test ipc_manager_test.cpp)
add_test(
  NAME
    ipc_manager_test
  COMMAND
    ipc_manager_test
)
target_include_directories(ipc_manager_test PRIVATE $<BUILD_INTERFACE:${SQLITE_MODERN_CPP_INCLUDE_DIRS}>)
target_link_libraries(ipc_manager_test PRIVATE tfc::ipc tfc::confman Boost::ut glaze::glaze unofficial::sqlite3::sqlite3)

add_executable(ipc_test ipc_test.cpp)
target_link_libraries(ipc_test PRIVATE Boost::ut tfc::ipc tfc::base)
add_test(NAME ipc_test COMMAND ipc_test)

add_executable(item_test item_test.cpp)
target_link_libraries(item_test PRIVATE Boost::ut tfc::ipc tfc::base tfc::stx)
add_test(NAME item_test COMMAND item_test)

add_executable(filter_test filter_test.cpp)
target_link_libraries(filter_test PRIVATE Boost::ut tfc::ipc tfc::base tfc::testing tfc::stub_confman)
add_test(NAME filter_test COMMAND filter_test)

add_executable(generate_python_unit_test generate_python_unit_test.cpp)
target_link_libraries(generate_python_unit_test PRIVATE tfc::ipc)
