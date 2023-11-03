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

