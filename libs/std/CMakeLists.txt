project(mstd)

add_library(mstd)
add_library(tfc::mstd ALIAS mstd)
target_sources(mstd
  PUBLIC FILE_SET CXX_MODULES FILES
    std.cxx
)
target_compile_features(mstd PUBLIC cxx_std_20)

#install(TARGETS std EXPORT CXXModules)