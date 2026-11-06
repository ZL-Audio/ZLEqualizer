# Keep shared sources at the root of the IDE project.
set_target_properties(SharedCode PROPERTIES FOLDER "")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/source" PREFIX "" FILES ${SourceFiles})

# Enable Xcode schemes only for the available plugin targets.
foreach (format IN ITEMS ${FORMATS} All)
    set(plugin_target "${PROJECT_NAME}_${format}")
    if (NOT TARGET "${plugin_target}")
        continue()
    endif ()

    set_target_properties("${plugin_target}" PROPERTIES
            FOLDER "Targets"
            XCODE_GENERATE_SCHEME ON
    )

    # Build JUCE's AudioPluginHost separately before launching these schemes.
    if (NOT format STREQUAL "All" AND NOT format STREQUAL "Standalone")
        set_target_properties("${plugin_target}" PROPERTIES
                XCODE_SCHEME_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app"
        )
    endif ()
endforeach ()

if (TARGET Assets)
    set_target_properties(Assets PROPERTIES FOLDER "Targets")
endif ()
