find_package(ut CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")

add_executable(themis_database_test themis_database_test.cpp)
target_link_libraries(themis_database_test
    PRIVATE
    Boost::ut
    tfc::base
    tfc::snitch
    unofficial::sqlite3::sqlite3
    OpenSSL::Crypto
)

target_include_directories(themis_database_test
    PRIVATE
    ../inc
)

add_executable(themis_integration_test themis_integration_test.cpp)
target_link_libraries(themis_integration_test
    PRIVATE
    Boost::ut
    tfc::base
    tfc::snitch
    unofficial::sqlite3::sqlite3
    OpenSSL::Crypto
)

target_include_directories(themis_integration_test
    PRIVATE
    ../inc
)

add_test(NAME themis_database_test COMMAND themis_database_test)
add_test(NAME themis_integration_test COMMAND themis_integration_test)