vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO openbmc/sdbusplus
  REF 1caa5e8a5ad9e9f26ddf2effe095ba1670911bc1
  SHA512 9b674457a79376ba2df0a52f07d8eea61720ca7635489cb7da1c0f7b218cddeae0ce95dabc2e90cf562eddbf28b724cd90ef6b321fa738b4f1ae60912518160a
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
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
