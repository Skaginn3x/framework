set(CXX_STANDARD_REQUIRED ON)

option(USE_EMULATION "use emulation of module 'std' instead of the library-provided one" OFF)

add_library(std STATIC)
add_library(mod::std ALIAS std)
# the default PMIF is 'std.ixx'
set(pmif std.ixx)

set(std-lang-additional -fsized-deallocation -faligned-allocation)

target_sources(std
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      FILES ${pmif}
)
target_compile_features(std PUBLIC cxx_std_23)

target_compile_definitions(std PRIVATE _BUILD_STD_MODULE)
target_compile_options(std PUBLIC ${std-lang-additional} PRIVATE -Wno-reserved-module-identifier)