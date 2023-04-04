# get glibc version from host
execute_process(
    COMMAND bash -c "ldd --version | awk '/ldd/{print $NF;exit}'"
    OUTPUT_VARIABLE LIBC_VERSION
    RESULT_VARIABLE LDD_RESULT
)

if (LDD_RESULT EQUAL 0)
  message("Host libc version is: ${LIBC_VERSION}")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-dbg")
endif()
set(CPACK_PACKAGE_VENDOR Skaginn 3X)
set(CPACK_PACKAGE_CONTACT "Skaginn 3X <software@skaginn3x.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
#set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME}) # todo
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
# professional cmake suggests this to be explicitly set to true because it defaults to FALSE for backwards compatibility.
set(CPACK_VERBATIM_VARIABLES YES)

set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation prefix" FORCE)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

# set dependencies to hosts libc version and cockpit for UI
if (LDD_RESULT EQUAL 0)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "cockpit, libc6 (>= ${LIBC_VERSION})")
  set(CPACK_RPM_PACKAGE_REQUIRES "cockpit, glibc >= ${LIBC_VERSION}")
else ()
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "cockpit")
  set(CPACK_RPM_PACKAGE_SUGGESTS "cockpit")
endif ()

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE x86_64)
set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64)

# TODO graphical installer properties
#set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_LIST_DIR}/Description.txt)
#set(CPACK_RESOURCE_FILE_WELCOME ${CMAKE_CURRENT_LIST_DIR}/Welcome.txt)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/../LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_LIST_DIR}/../readme.md)

include(CPack)
