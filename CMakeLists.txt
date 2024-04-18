# 3.24.1 is bundled in Visual Studio 2022 v17.4
# 3.24.1 is also bundled in CLion as of 2023
cmake_minimum_required(VERSION 3.24.1)

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-includes")
include(GitVersion)
include(PamplejuceVersion)

# Change me!
# This is the internal name of the project and the name of JUCE's shared code "target"
# Note: This cannot have spaces (it may be 2023, but you can't have it all!)
# Worry not, JUCE's PRODUCT_NAME can have spaces (and is DAWs display)
set(PROJECT_NAME "ZLEqualizer")
set(PLUGIN_CODE "Equa")

# Worry not, JUCE's PRODUCT_NAME can have spaces (and is what DAWs will display)
# You can also just have it be the same thing as PROJECT_NAME
# You may want to append the major version on the end of this (and PROJECT_NAME) ala:
# #  set(PROJECT_NAME "MyPlugin_v${MAJOR_VERSION}")
# Doing so enables major versions to show up in IDEs and DAWs as separate plugins
# allowing you to change parameters and behavior without breaking existing user projects
set(PRODUCT_NAME "ZL Equalizer")

# Change me! Used for the MacOS bundle name and Installers
set(COMPANY_NAME "ZL")
set(PLUGIN_MANUFACTURER_CODE "Zliu")

# Change me! Used for the MacOS bundle identifier (and signing)
set(BUNDLE_ID "com.zlaudio.plugins")

set(LV2URI "https://github.com/ZL-Audio/${PROJECT_NAME}")

# For simplicity, the name of the CMake project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# Change me! Set the plugin formats you want built
# Valid choices: AAX Unity VST VST3 AU AUv3 Standalone
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(FORMATS AU VST3)
else ()
    set(FORMATS LV2 VST3)
endif ()

# include boost for this project
find_package(Boost 1.80.0 REQUIRED)

# Configures universal binaries in CI
include(PamplejuceMacOS)

# Couple tweaks that IMO should be JUCE defaults
include(JUCEDefaults)

# JUCE is setup as a submodule in the /JUCE folder
# Locally, you must run `git submodule update --init --recursive` once
# and later `git submodule update --remote --merge` to keep it up to date
# On Github Actions, this is done as a part of actions/checkout
add_subdirectory(JUCE)

# Add any other modules you want modules here, before the juce_add_plugin call
juce_add_module("${CMAKE_CURRENT_LIST_DIR}/modules/friz/Source/friz")

# See `docs/CMake API.md` in the JUCE repo for all config options
juce_add_plugin("${PROJECT_NAME}"
        # Icons for the standalone app
        # ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"

        # Change me!
        COMPANY_NAME "${COMPANY_NAME}"
        BUNDLE_ID "${BUNDLE_ID}"

        # On MacOS, plugin is copied to ~/Users/yourname/Library/Audio/Plug-Ins/
        COPY_PLUGIN_AFTER_BUILD TRUE

        # Change me!
        # A four-character plugin id
        # First character MUST be uppercase for AU formats
        PLUGIN_MANUFACTURER_CODE "${PLUGIN_MANUFACTURER_CODE}"

        # Change me!
        # A unique four-character plugin id
        # Note: this must have at least one upper-case character
        PLUGIN_CODE "${PLUGIN_CODE}"
        FORMATS "${FORMATS}"
        LV2URI "${LV2URI}"

        # The name of your final executable
        # This is how it's listed in the DAW
        # This can be different from PROJECT_NAME and can have spaces!
        # You might want to use v${MAJOR_VERSION} here once you go to v2...
        PRODUCT_NAME "${PRODUCT_NAME}")

# This lets us use our code in both the JUCE targets and our Test target
# Without running into ODR violations
add_library(SharedCode INTERFACE)

# C++20, please
# Use cxx_std_23 for C++23 (as of CMake v 3.20)
target_compile_features(SharedCode INTERFACE cxx_std_20)

# Manually list all .h and .cpp files for the plugin
# If you are like me, you'll use globs for your sanity.
# Just ensure you employ CONFIGURE_DEPENDS so the build system picks up changes
# If you want to appease the CMake gods and avoid globs, manually add files like so:
# set(SourceFiles Source/PluginEditor.h Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginProcessor.cpp)
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
target_sources(SharedCode INTERFACE ${SourceFiles})

# Adds a BinaryData target for embedding assets into the binary
include(Assets)

# MacOS only: Cleans up folder and target organization on Xcode.
include(XcodePrettify)

# This is where you can set preprocessor definitions for JUCE and your plugin
target_compile_definitions(SharedCode
        INTERFACE

        # JUCE_WEB_BROWSER and JUCE_USE_CURL off by default
        JUCE_WEB_BROWSER=0  # If you set this to 1, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you set this to 1, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0

        # lets the app known if we're Debug or Release
        CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        VERSION="${CURRENT_VERSION}"

        # JucePlugin_Name is for some reason doesn't use the nicer PRODUCT_NAME
        PRODUCT_NAME_WITHOUT_VERSION="${PRODUCT_NAME}"
        #        JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS=1
        JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
)

# Link to any other modules you added (with juce_add_module) here!
# Usually JUCE modules must have PRIVATE visibility
# See https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md#juce_add_module
# However, with Pamplejuce, you'll link modules to SharedCode with INTERFACE visibility
# This allows the JUCE plugin targets and the Tests target to link against it
target_link_libraries(SharedCode
        INTERFACE
        Assets
        juce_audio_utils
        juce_audio_processors
        juce_dsp
        juce_gui_basics
        juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        friz
        Boost::boost)

# Link the JUCE plugin targets our SharedCode target
target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)

# IPP support, comment out to disable
include(PamplejuceIPP)

# Everything related to the tests target
# include(Tests)

# A separate target keeps the Tests target fast!
# include(Benchmarks)

# Pass some config to GA (like our PRODUCT_NAME)
include(GitHubENV)
