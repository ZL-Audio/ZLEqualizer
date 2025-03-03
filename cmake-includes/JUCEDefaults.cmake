# Adds all the module sources so they appear correctly in the IDE
# Must be set before JUCE is added as a sub-dir (or any targets are made)
# https://github.com/juce-framework/JUCE/commit/6b1b4cf7f6b1008db44411f2c8887d71a3348889
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# Creates a /Modules directory in the IDE with the JUCE Module code
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

# Static runtime please
# See https://github.com/sudara/pamplejuce/issues/111
if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
endif ()

# Color our warnings and errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif ()

# Checkout JUCE version
if (("$ENV{JUCE_BRANCH}" STREQUAL "juce7") AND GIT_EXECUTABLE)
    execute_process(
            COMMAND "${GIT_EXECUTABLE}" checkout "fd933dfac612fcb0bbb1443395da3b5b289604f4"
            RESULT_VARIABLE RES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/JUCE
    )
    if (NOT RES EQUAL 0)
        message(FATAL_ERROR "Failed to check out JUCE 7")
    else ()
        message(STATUS "Check out JUCE 7")
    endif ()
endif ()