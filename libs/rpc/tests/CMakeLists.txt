find_package(ut CONFIG REQUIRED)

add_executable(json_rpc json_rpc.cpp)

target_link_libraries(json_rpc PRIVATE tfc::rpc Boost::ut glaze::glaze)

add_test(
  NAME
  json_rpc
  COMMAND
  json_rpc
)

add_subdirectory(examples)