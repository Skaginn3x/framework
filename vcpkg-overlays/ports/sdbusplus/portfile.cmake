vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO skaginn3x/sdbusplus
  REF 725d6d57fc13d1a9a0bf795410adc765af896a88
  SHA512 e9ec3a7fba335ec0ad29c906a464c7997dd54717cb1e83802f70dc8f0f91a16ac9030beefb08fcd18d621c61bf6bba801c234df547d807eadf458fde434dcffd
  PATCHES
    libcpp-does-not-have-stop-token.patch # https://en.cppreference.com/w/cpp/20
)

x_vcpkg_get_python_packages(
        PYTHON_VERSION "3"
        PACKAGES mako pyyaml inflection
)

vcpkg_configure_meson(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -Dtests=disabled
    -Dexamples=disabled
)

vcpkg_install_meson()

vcpkg_fixup_pkgconfig()

configure_file("${CMAKE_CURRENT_LIST_DIR}/Config.cmake.in" "${CURRENT_PACKAGES_DIR}/share/unofficial-sdbusplus/unofficial-sdbusplus-config.cmake" @ONLY)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
