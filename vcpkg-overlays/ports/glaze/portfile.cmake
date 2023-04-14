vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stephenberry/glaze
    REF b52a7d56f17bf3f53d9c2288f59d4acff9cb4e9f
    SHA512 9957f7d482b6de424e14ee8e38560dea47ec0324b0968a5d9fb64b9d7f5309265c162c978205647ed30d05933230671851df06c39f01f6d86dd28747802bee8c
    PATCHES
      disable-dev-mode.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/glaze RENAME copyright)
