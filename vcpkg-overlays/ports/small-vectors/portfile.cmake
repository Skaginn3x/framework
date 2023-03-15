vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO arturbac/small_vectors
    REF v1.0.17
    SHA512 ad9397d78d197222762f11e41791c22fc34b2fedad3dfa8bdf4d4ca5666976cc4e1a35ab7af4b7fd9feca6387b009261c4bd5bd093eecb9a02c6609940749f0a
    PATCHES
        disable-cpm.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DSMALL_VECTORS_ENABLE_UNIT_TESTS=OFF
        -DINCLUDE_INSTALL_DIR=include
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/small-vectors RENAME copyright)
