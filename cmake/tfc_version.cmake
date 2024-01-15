# Run this with CMake in "script mode" (-P flag) at build time

# find Git and if available set GIT_HASH variable
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
            COMMAND git log -1 --pretty=format:%H
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND git log -1 --pretty=format:"%an <%ae>"
            OUTPUT_VARIABLE GIT_AUTHOR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND git branch --show-current
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND git describe --tags --abbrev=1
            OUTPUT_VARIABLE GIT_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND git diff --shortstat
            OUTPUT_VARIABLE GIT_IS_DIRTY_INTERMEDIATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND git log -1 --pretty=format:%as
            OUTPUT_VARIABLE GIT_COMMIT_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    execute_process(
            COMMAND date +"%Y-%M-%d %H:%M:%S"
            OUTPUT_VARIABLE BUILD_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
else()
    message(FATAL_ERROR "Git not found, cannot generate version.hpp")
endif()
# Write git dirty if GIT_DIRTY_INTERMEDIATE is not empty
if(NOT "${GIT_IS_DIRTY_INTERMEDIATE}" STREQUAL "")
    set(GIT_IS_DIRTY "dirty")
else()
    set(GIT_IS_DIRTY "")
endif()

# Print all git fields
message(STATUS "GIT_HASH: ${GIT_HASH}")
message(STATUS "GIT_AUTHOR: ${GIT_AUTHOR}")
message(STATUS "GIT_BRANCH: ${GIT_BRANCH}")
message(STATUS "GIT_TAG: ${GIT_TAG}")
message(STATUS "GIT_IS_DIRTY: ${GIT_IS_DIRTY}")
message(STATUS "GIT_COMMIT_DATE: ${GIT_COMMIT_DATE}")

# generate file version.hpp based on version.hpp.in
configure_file(
        ${IN_FILE}
        ${OUT_FILE}
        @ONLY
)
