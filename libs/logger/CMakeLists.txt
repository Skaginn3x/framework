project(logger)

add_library(logger
  src/logger.cpp
)
add_library(tfc::logger ALIAS logger)
target_include_directories(logger
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
)

find_package(Boost REQUIRED COMPONENTS)
target_link_libraries(logger
  PUBLIC
    mod::fmt
    mod::spdlog
    mod::asio
    Boost::boost
  PRIVATE
    tfc::base
    tfc::stx
)

add_library_to_docs(tfc::logger)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

include(tfc_install)
tfc_install_lib(logger)
