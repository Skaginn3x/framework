set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_FLAGS "")
set(LINK_FLAGS "-fuse-ld=lld")

set(ENV{CC} gcc)
set(ENV{CXX} g++)
set(ENV{CFLAGS} "")
set(ENV{LDFLAGS} -fuse-ld=lld)

set(VCPKG_FIXUP_ELF_RPATH ON)
