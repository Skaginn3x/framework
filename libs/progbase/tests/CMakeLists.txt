find_package(ut CONFIG REQUIRED)

add_executable(progbase_options_test options.cpp)
target_link_libraries(progbase_options_test Boost::ut tfc::base)

add_test(NAME progbase_options_test COMMAND progbase_options_test)

# Add examples to tests so that they definitely compile
add_executable(progbase_example_base examples/example_base.cpp)
target_link_libraries(progbase_example_base tfc::base Boost::program_options)
add_test(
  NAME
    progbase_example_base
  COMMAND
    progbase_example_base
)

# Tell doxygen where to find this example
add_example_to_docs(progbase_example_base)