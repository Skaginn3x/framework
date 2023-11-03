set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
# Libc++ doesn't support compiler extensions for modules.
set(CMAKE_CXX_EXTENSIONS OFF)


set(LIBCXX_BUILD /cpproot)
add_compile_options(-Wno-reserved-identifier)
## import std;

include(FetchContent)
FetchContent_Declare(
    std
    URL "file://${LIBCXX_BUILD}/modules/c++/v1/"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(std)
if(NOT std_POPULATED)
  FetchContent_Populate(std)
  add_subdirectory(${std_SOURCE_DIR} ${std_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

#
# Adjust project compiler flags
#

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fprebuilt-module-path=${CMAKE_BINARY_DIR}/_deps/std-build/CMakeFiles/std.dir/>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-nostdinc++>)
# The include path needs to be set to be able to use macros from headers.
# For example from, the headers <cassert> and <version>.
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-isystem>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${LIBCXX_BUILD}/include/c++/v1>)

#
# Adjust project linker flags
#

add_link_options($<$<COMPILE_LANGUAGE:CXX>:-nostdlib++>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-L${LIBCXX_BUILD}/lib>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-Wl,-rpath,${LIBCXX_BUILD}/lib>)
# Linking against std is required for CMake to get the proper dependencies
link_libraries(std c++)