set(CMAKE_SYSTEM_PROCESSOR x64)
set(CMAKE_C_COMPILER /opt/clang-16.0.0/bin/clang)
set(CMAKE_CXX_COMPILER /opt/clang-16.0.0/bin/clang++)
set(CMAKE_CXX_FLAGS "-stdlib=libc++ -I/opt/clang-16.0.0/include/")
set(CMAKE_C_FLAGS "")
set(LINK_FLAGS -fuse-ld=lld)
set(STATIC_LIBRARY_FLAGS -fuse-ld=lld)
