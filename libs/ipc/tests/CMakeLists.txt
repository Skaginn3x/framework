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

add_subdirectory(examples)
