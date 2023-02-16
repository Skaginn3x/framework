project(confman)

add_executable(confman
    src/main.cpp
)
target_link_libraries(confman
    PUBLIC
        tfc::confman
)