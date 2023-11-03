## ARGPARSE
add_compile_options(-Wno-include-angled-in-module-purview)
find_package(argparse CONFIG REQUIRED)
add_library(argparse_mod)
target_compile_definitions(argparse_mod PUBLIC ARGPARSE_MODULE_USE_STD_MODULE)
add_library(mod::argparse ALIAS argparse_mod)
target_sources(argparse_mod
  PUBLIC FILE_SET CXX_MODULES FILES
    argparse.cxx
)
target_link_libraries(argparse_mod PUBLIC argparse::argparse std)

find_package(fmt CONFIG REQUIRED)
add_library(fmt_mod)
add_library(mod::fmt ALIAS fmt_mod)
target_sources(fmt_mod
    PUBLIC FILE_SET CXX_MODULES FILES
    fmt.cxx
)
target_link_libraries(fmt_mod PUBLIC fmt::fmt std)

find_package(spdlog CONFIG REQUIRED)
add_library(spdlog_mod)
add_library(mod::spdlog ALIAS spdlog_mod)
target_sources(spdlog_mod
    PUBLIC FILE_SET CXX_MODULES FILES
    spdlog.cxx
)
target_link_libraries(spdlog_mod PUBLIC spdlog::spdlog mod::fmt std)

find_package(asio REQUIRED )
add_library(asio_mod)
add_library(mod::asio ALIAS asio_mod)
target_sources(asio_mod
    PUBLIC FILE_SET CXX_MODULES FILES
    asio.cxx
)
target_link_libraries(asio_mod PUBLIC asio::asio std)
