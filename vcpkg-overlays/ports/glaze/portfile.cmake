vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stephenberry/glaze
    REF v0.3.6
    SHA512 80e5dd02f1bc41458aa483affed8a1a00320d2301ee7ba32bb87aebf02e9a0db19ac748d2252f2a9a9756ccd6b0af5cb673070730b7a9e4bf6e1f9aafaf05447
    PATCHES
      disable-dev-mode.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/glaze RENAME copyright)
