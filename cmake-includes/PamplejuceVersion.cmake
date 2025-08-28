# Reads in our VERSION file and sticks in it CURRENT_VERSION variable
# Be sure the file has no newlines!
# This exposes CURRENT_VERSION to the build system
# And it's later fed to JUCE so it shows up as VERSION in your IDE
file(STRINGS VERSION CURRENT_VERSION)

# Figure out the major version to append to our PROJECT_NAME
string(REGEX MATCH "([0-9]+)" MAJOR_VERSION ${CURRENT_VERSION})
message(STATUS "Major version: ${MAJOR_VERSION}")
