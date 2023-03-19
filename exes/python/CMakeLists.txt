add_executable(tfcpython main.cpp)

find_package(Python3 COMPONENTS Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(tfcpython
  PRIVATE
    tfc::base
    tfc::logger
    Boost::boost
    Boost::program_options
    Python3::Python
    pybind11::lto
    pybind11::embed
    pybind11::module
    pybind11::headers
)


