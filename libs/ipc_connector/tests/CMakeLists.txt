
find_package(ut CONFIG REQUIRED)

add_executable(ipc_connector_test ipc_connector_test.cpp)
target_link_libraries(ipc_connector_test PRIVATE Boost::ut tfc::ipc_connector)

add_test(NAME ipc_connector_test COMMAND ipc_connector_test)
