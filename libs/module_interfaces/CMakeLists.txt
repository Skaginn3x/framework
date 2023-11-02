add_subdirectory(std.module-module)
## ARGPARSE
add_compile_definitions(ARGPARSE_MODULE_USE_STD_MODULE)
find_package(argparse CONFIG REQUIRED)
add_library(argparse_mod)
add_library(mod::argparse ALIAS argparse_mod)
target_sources(argparse_mod
  PUBLIC FILE_SET CXX_MODULES FILES
    argparse.cxx
)
target_link_libraries(argparse_mod PUBLIC argparse::argparse mod::std)

