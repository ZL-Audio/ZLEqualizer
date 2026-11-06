# Every file under assets is embedded in the plugin binary.
file(GLOB_RECURSE AssetFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
juce_add_binary_data(Assets SOURCES ${AssetFiles})

# The generated assets are linked into shared plugin libraries.
set_target_properties(Assets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
