vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stephenberry/glaze
    REF v0.3.6
    SHA512 0
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
#vcpkg_fixup_cmake_targets()

file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/glaze RENAME copyright)
