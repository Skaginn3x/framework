vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO dankamongmen/notcurses
    REF v${VERSION}
    SHA512 e867d2436f7c953b4b7feb1464b73709cb792256e82956c933c43981dad802c30526d53d28ebafd8e460a3309ae4895cac4e0d1f6f89e347ab9578546798d19b
    HEAD_REF master
)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" NOTCURSES_BUILD_STATIC)
vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
    -DBUILD_TESTING=OFF
    -DUSE_DEFLATE=OFF
    -DUSE_PANDOC=OFF
    -DUSE_STATIC=${NOTCURSES_BUILD_STATIC}
)
vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/Notcurses++)
vcpkg_fixup_pkgconfig()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYRIGHT")
