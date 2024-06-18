find_package(ut CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS gmock)
find_path(BEXT_SML_INCLUDE_DIRS "boost/sml.hpp")

add_executable(themis_database_test themis_database_test.cpp)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")

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