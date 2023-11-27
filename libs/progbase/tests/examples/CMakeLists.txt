# Add examples to tests so that they definitely compile
add_executable(progbase_example_base example_base.cpp)
target_link_libraries(progbase_example_base tfc::base Boost::program_options)
add_test(
        NAME
        progbase_example_base
        COMMAND
        progbase_example_base
)

# Tell doxygen where to find this example
add_example_to_docs(progbase_example_base)