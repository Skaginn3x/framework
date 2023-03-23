vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stephenberry/glaze
    REF df0e272f7c62bd2bb86e487c65ffeea56eb79ab7
    SHA512 e01a603d0db3b02c4935835703ffa8da24be0c464ccbe161b735517591aa61ef00b4815c8f02a0b93971bc9f7d5bfec491a8a5c21e8f5f3998d87e48943c6fcb
    PATCHES
      disable-dev-mode.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/glaze RENAME copyright)
