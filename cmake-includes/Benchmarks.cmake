file(GLOB_RECURSE BenchmarkFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Benchmarks/*.h")

# Organize the test source in the Tests/ folder in the IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks PREFIX "" FILES ${BenchmarkFiles})

add_executable(Benchmarks ${BenchmarkFiles})
target_compile_features(Benchmarks PRIVATE cxx_std_20)
catch_discover_tests(Benchmarks)

# Our benchmark executable also wants to know about our plugin code...
target_include_directories(Benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Copy over compile definitions from our plugin target so it has all the JUCEy goodness
target_compile_definitions(Benchmarks PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME},COMPILE_DEFINITIONS>)

# And give tests access to our shared code
target_link_libraries(Benchmarks PRIVATE SharedCode Catch2::Catch2WithMain)

# Make an Xcode Scheme for the test executable so we can run tests in the IDE
set_target_properties(Benchmarks PROPERTIES XCODE_GENERATE_SCHEME ON)

# When running Tests we have specific needs
target_compile_definitions(Benchmarks PUBLIC
    JUCE_MODAL_LOOPS_PERMITTED=1 # let us run Message Manager in tests
    RUN_PAMPLEJUCE_TESTS=1 # also run tests in module .cpp files guarded by RUN_PAMPLEJUCE_TESTS
)
