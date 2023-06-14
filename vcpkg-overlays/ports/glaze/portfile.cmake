if(VCPKG_TARGET_IS_LINUX)
  message("Warning: `glaze` requires Clang or GCC 10+ on Linux")
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stephenberry/glaze
    HEAD_REF main
    REF 01270bc86f84c46854ebd706cd44a1cee09e2dd7
    SHA512 2c8fa0b84082933d5dd5dcb259d8b04a8ef63a63474f300aa5915d5aa1230223248ac217028a10aa458d956603280686197549f84744eb562f822e427485f6f6
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
    -Dglaze_DEVELOPER_MODE=OFF
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
