vcpkg_from_gitlab(
    GITLAB_URL https://gitlab.com
    OUT_SOURCE_PATH SOURCE_PATH
    REPO etherlab.org/ethercat
    REF 7f56fce837e15a863168dd4ea12c6b44f95911ef
    SHA512 2bfb0652053a68179fe0bc84532855b781e11ff953c09e819172fb85a995466b9c0b473c4eecfa0155fb5cd3dfb9acadf3e6cb465011d61a305eb495803518de
)

vcpkg_configure_make(
    SOURCE_PATH "${SOURCE_PATH}/lib"
)

vcpkg_make_install()

add_library()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/ethercat RENAME copyright)
