
add_library(confman_mocks INTERFACE)
add_library(tfc::mocks::confman ALIAS confman_mocks)

target_include_directories(confman_mocks
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc/public>
    $<INSTALL_INTERFACE:inc/public>
)

find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
target_link_libraries(confman_mocks
  INTERFACE
    tfc::confman
    GTest::gmock
)

add_library_to_docs(tfc::mocks::confman)

add_executable(testme test.cpp)

find_package(ut CONFIG REQUIRED)
target_link_libraries(testme
  PUBLIC
    tfc::mocks::confman
    Boost::ut
)
