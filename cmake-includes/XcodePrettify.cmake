# No, we don't want our source buried in extra nested folders
set_target_properties(SharedCode PROPERTIES FOLDER "")

# The Xcode source tree should uhhh, still look like the source tree, yo
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "" FILES ${SourceFiles})

# It tucks the Plugin varieties into a "Targets" folder and generate an Xcode Scheme manually
# Xcode scheme generation is turned off globally to limit noise from other targets
# The non-hacky way of doing this is via the global PREDEFINED_TARGETS_FOLDER property
# However that doesn't seem to be working in Xcode
# Not all plugin types (au, vst) available on each build type (win, macos, linux)
foreach (target ${FORMATS} "All")
    if (TARGET ${PROJECT_NAME}_${target})
        set_target_properties(${PROJECT_NAME}_${target} PROPERTIES
            # Tuck the actual plugin targets into a folder where they won't bother us
            FOLDER "Targets"
            # Let us build the target in Xcode
            XCODE_GENERATE_SCHEME ON)

        # Set the default executable that Xcode will open on build
        # Note: you must manually build the AudioPluginHost.xcodeproj in the JUCE subdir
        if ((NOT target STREQUAL "All") AND (NOT target STREQUAL "Standalone"))
            set_target_properties(${PROJECT_NAME}_${target} PROPERTIES
                XCODE_SCHEME_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app")
        endif ()
    endif ()
endforeach ()

if (TARGET Assets)
    set_target_properties(Assets PROPERTIES FOLDER "Targets")
endif ()
