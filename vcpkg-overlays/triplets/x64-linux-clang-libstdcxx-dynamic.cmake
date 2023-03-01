set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
set(CMAKE_C_FLAGS "")
set(LINK_FLAGS "-fuse-ld=lld")

set(ENV{CC} clang)
set(ENV{CXX} clang++)
set(ENV{CXXFLAGS} -stdlib=libstdc++)
set(ENV{CFLAGS} "")
set(ENV{LDFLAGS} -fuse-ld=lld)

set(VCPKG_FIXUP_ELF_RPATH ON)
