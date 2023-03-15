
find_package(ut CONFIG REQUIRED)

add_executable(ipc_test ipc_test.cpp)
target_link_libraries(ipc_test PRIVATE Boost::ut tfc::ipc tfc::base)

add_test(NAME ipc_test COMMAND ipc_test)
