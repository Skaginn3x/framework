set(CMAKE_CXX_STANDARD 23)

# Disable boost warning on new version
set(Boost_NO_WARN_NEW_VERSIONS 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # enable full llvm assembler, byte code gen, linking optimisations
  add_compile_options( -fuse-ld=lld )
  add_link_options( -fuse-ld=lld )
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options( -flto=thin )
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(
      -fvisibility=hidden
      -fvisibility-inlines-hidden
  )
else()
  add_compile_options(-fvisibility=default)
endif()

if(ENABLE_STATIC_LINKING)
  add_link_options(-static-libstdc++ -static-libgcc)
endif ()
