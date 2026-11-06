# Resolve build metadata before project() uses CURRENT_VERSION.
set(CURRENT_VERSION "0.0.0")
set(CURRENT_HASH "0000000")
find_package(Git QUIET)

if (Git_FOUND)
    execute_process(
            COMMAND "${GIT_EXECUTABLE}" describe --tags --abbrev=0
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
            RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    if (GIT_DESCRIBE_ERROR_CODE STREQUAL "0")
        string(REGEX REPLACE "^v" "" CURRENT_VERSION "${GIT_DESCRIBE_VERSION}")
    else ()
        message(STATUS "No Git version tag found; using ${CURRENT_VERSION}.")
    endif ()

    execute_process(
            COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_CURRENT_HASH
            RESULT_VARIABLE GIT_HASH_ERROR_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    if (GIT_HASH_ERROR_CODE STREQUAL "0")
        set(CURRENT_HASH "${GIT_CURRENT_HASH}")
    else ()
        message(STATUS "No Git commit found; using ${CURRENT_HASH}.")
    endif ()
else ()
    message(STATUS "Git not found; using default version and hash.")
endif ()

string(REGEX MATCH "^[0-9]+" MAJOR_VERSION "${CURRENT_VERSION}")
message(STATUS "Plugin version: ${CURRENT_VERSION} (${CURRENT_HASH})")

# Keep the generated VERSION file available to packaging tools.
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" "${CURRENT_VERSION}")
add_compile_definitions(
        ZL_PLUGIN_CURRENT_VERSION="${CURRENT_VERSION}"
        ZL_PLUGIN_CURRENT_HASH="${CURRENT_HASH}"
)
