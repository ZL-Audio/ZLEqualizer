# Read version from git tag and write it to VERSION file
find_package(Git)

if (GIT_EXECUTABLE)
    # Generate a git-describe version string from Git repository tags
    execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
            OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
            RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT GIT_DESCRIBE_ERROR_CODE)
        set(FOOBAR_VERSION ${GIT_DESCRIBE_VERSION})
        message(STATUS "Using the version \"${FOOBAR_VERSION}\".")
    endif ()
    execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
            OUTPUT_VARIABLE GIT_CURRENT_HASH
            RESULT_VARIABLE GIT_HASH_ERROR_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT GIT_HASH_ERROR_CODE)
        set(FOOBAR_HASH ${GIT_CURRENT_HASH})
    endif ()
endif ()

if (NOT DEFINED FOOBAR_VERSION)
    set(FOOBAR_VERSION 0.0.0)
    message(WARNING "Failed to determine VERSION from Git tags. Using default version \"${FOOBAR_VERSION}\".")
endif ()
if (NOT DEFINED FOOBAR_HASH)
  set(FOOBAR_HASH "0000000")
  message(WARNING "Failed to determine HASH from Git logs. Using default hash \"${FOOBAR_HASH}\".")
endif ()

string(REPLACE "v" "" FOOBAR_VERSION_WITHOUT_V "${FOOBAR_VERSION}")

file(WRITE VERSION "${FOOBAR_VERSION_WITHOUT_V}")

add_definitions(-DZLEQUALIZER_CURRENT_VERSION="${FOOBAR_VERSION_WITHOUT_V}")

add_definitions(-DZLEQUALIZER_CURRENT_HASH="${FOOBAR_HASH}")